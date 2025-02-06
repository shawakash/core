#include "http.hpp"
#include "config.cpp"
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <vector>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Events
struct PriceUpdateEvent {
    std::string ticker;
    double lastPrice;
    double percentChange;
};

struct VolumeUpdateEvent {
    std::string ticker;
    double volume;
    double percentChange;
};

// Observer Interfaces
class PriceObserver {
    public:
        virtual void updatePrice(const PriceUpdateEvent& priceUpdate) = 0;
        virtual ~PriceObserver() = default;
};

class VolumeObserver {
    public:
        virtual void updateVolume(const VolumeUpdateEvent& volumeUpdate) = 0;
        virtual ~VolumeObserver() = default;
};

// Ticker Details
struct TickerDetails {
    double volume;
    double lastPrice;
    double volumePercentChange = 0.0;
    double pricePercentChange = 0.0;
};

// Ticker Monitor (Thresholds for notification)
struct TickerMonitor {
    double maxVolumePercentChange;
    double maxPricePercentChange;
};

// Binance Subject (Observable)
class BinanceSubject {
    private:
        // Map to hold ticker data; key is ticker symbol.
        std::unordered_map<std::string, TickerDetails> tickerDetails;

        std::vector<PriceObserver*> pObserver;
        std::vector<VolumeObserver*> vObserver;

        HttpClient http;

        // Map to hold ticker threshold values; key is ticker symbol.
        std::unordered_map<std::string, TickerMonitor> tm;
        bool running = true;

        void notifyPriceObserver(const PriceUpdateEvent& priceEvent) {
            for (PriceObserver* observer : pObserver) {
                observer->updatePrice(priceEvent);
            }
        }

        void notifyVolumeObserver(const VolumeUpdateEvent& volumeEvent) {
            for (VolumeObserver* observer : vObserver) {
                observer->updateVolume(volumeEvent);
            }
        }

        TickerDetails fetchTickerDetails(const std::string& ticker) {
            try {
                const std::string tickerUrl = url + "?symbol=" + ticker;

                HttpRequestOptions options;
                options.headers = {
                    {"Accept", "application/json"}
                };

                HttpResponse res = http.fetch(tickerUrl, options);

                if (res.statusCode != 200) {
                    std::cerr << "[ERROR] Fetch failed for " << ticker << " - HTTP Code: " << res.statusCode << std::endl;
                    return TickerDetails();
                }

                // Marshall the res.body
                json data = json::parse(res.body);

                double volume = std::stod(data["volume"].get<std::string>());
                double lastPrice = std::stod(data["lastPrice"].get<std::string>());

                TickerDetails details;
                details.lastPrice = lastPrice;
                details.volume = volume;

                if (tickerDetails.find(ticker) != tickerDetails.end()) {
                    const TickerDetails& prev = tickerDetails[ticker];
                    details.volumePercentChange = ((volume - prev.volume) / prev.volume) * 100.0;
                    details.pricePercentChange = ((lastPrice - prev.lastPrice) / prev.lastPrice) * 100.0;
                }

                return details;

            } catch (const std::exception& e) {
                std::cerr << "[ERROR] Exception fetching ticker details: " << e.what() << std::endl;
                return TickerDetails();
            }
        }

        // Checks if the ticker's changes exceed thresholds and notifies observers.
        void checkForNotif(const std::string& ticker) {
            // Ensure we have both ticker details and threshold data.
            if (tickerDetails.find(ticker) == tickerDetails.end() ||
                tm.find(ticker) == tm.end()) {
                return;
            }

            const TickerDetails& details = tickerDetails[ticker];
            const TickerMonitor& monitor = tm[ticker];

            if (details.volumePercentChange >= monitor.maxVolumePercentChange) {
                VolumeUpdateEvent volumeUpdate{ ticker, details.volume, details.volumePercentChange };
                notifyVolumeObserver(volumeUpdate);
            }

            if (details.pricePercentChange >= monitor.maxPricePercentChange) {
                PriceUpdateEvent priceUpdate{ ticker, details.lastPrice, details.pricePercentChange };
                notifyPriceObserver(priceUpdate);
            }
        }

        void setAllTickerDetails() {
            try {

                HttpRequestOptions options;
                options.headers = {{"Accept", "application/json"}};

                HttpResponse res = http.fetch(url, options);

                if (res.statusCode != 200) {
                    std::cerr << "[ERROR] Fetch failed for getAllTickerDetails - HTTP Code: " << res.statusCode << std::endl;
                    return;
                }

                json data = json::parse(res.body);

                for (const auto& detailsData : data) {
                    std::string ticker = detailsData["symbol"].get<std::string>();

                    // Skip if this ticker is not monitored.
                    if (tm.find(ticker) == tm.end()) {
                        continue;
                    }

                    double volume = std::stod(detailsData["volume"].get<std::string>());
                    double lastPrice = std::stod(detailsData["lastPrice"].get<std::string>());

                    if (tickerDetails.find(ticker) != tickerDetails.end()) {
                        const TickerDetails& prev = tickerDetails[ticker];
                        tickerDetails[ticker].volumePercentChange = (prev.volume != 0) ? ((volume - prev.volume) / prev.volume) * 100.0 : 0.0;
                        tickerDetails[ticker].pricePercentChange = (prev.lastPrice != 0) ? ((lastPrice - prev.lastPrice) / prev.lastPrice) * 100.0 : 0.0;

                    }

                    tickerDetails[ticker].volume = volume;
                    tickerDetails[ticker].lastPrice = lastPrice;

                    checkForNotif(ticker);
                }

            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                return;
            }
        }

        void monitorTickers() {
            while(running) {
                try {
                    std::cout << "\n....Monitor Tickers....\n" << std::endl;
                    setAllTickerDetails();

                    std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIME));
                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                    return;
                }
            }
        }


    public:
        std::string url;

        BinanceSubject(const std::string& apiUrl,
            const std::unordered_map<std::string, TickerMonitor>& tickers)
        : url(apiUrl), tm(tickers) {}

        void subscribePriceObserver(PriceObserver* observer) {
            pObserver.push_back(observer);
        }

        void subscribeVolumeObserver(VolumeObserver* observer) {
            vObserver.push_back(observer);
        }

        void unsubscribePriceObserver(PriceObserver* observer) {
            pObserver.erase(std::remove(pObserver.begin(), pObserver.end(), observer),
                pObserver.end());
        }

        void unsubscribeVolumeObserver(VolumeObserver* observer) {
            vObserver.erase(std::remove(vObserver.begin(), vObserver.end(), observer),
                vObserver.end());
        }

        void setUrl(const std::string& newUrl) {
            url = newUrl;
        }

        void updateTickerMonitor(const std::string& ticker, const TickerMonitor& tickerMonitor) {
            if(tm.find(ticker) == tm.end()) {
                std::cerr << "Error: such ticker is not being monitored" << std::endl;
                return;
            }

            tm[ticker] = tickerMonitor;
        }

        void run() {
            try {
                std::thread monitorThread(&BinanceSubject::monitorTickers, this);
                monitorThread.detach();
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] Exception in run(): " << e.what() << std::endl;
            }
        }

        void stop() {
            running = false;
        }

};

class DummyPriceObserver : public PriceObserver {
public:
    void updatePrice(const PriceUpdateEvent& event) override {
        std::cout << "[Price Update] Ticker: " << event.ticker
                  << ", Last Price: " << event.lastPrice
                  << ", Change: " << event.percentChange << "%" << std::endl;
    }
};

class DummyVolumeObserver : public VolumeObserver {
public:
    void updateVolume(const VolumeUpdateEvent& event) override {
        std::cout << "[Volume Update] Ticker: " << event.ticker
                  << ", Volume: " << event.volume
                  << ", Change: " << event.percentChange << "%" << std::endl;
    }
};

int main() {
    const std::string binanceUrl = "https://api.binance.com/api/v3/ticker/24hr";

    std::unordered_map<std::string, TickerMonitor> tickers = {
        {"BTCUSDT", TickerMonitor{0.00001, 0.00001}},
        {"ETHUSDT", TickerMonitor{0.00001, 0.00001}}
    };

    BinanceSubject binanceSubject(binanceUrl, tickers);

    DummyPriceObserver priceObs;
    DummyVolumeObserver volumeObs;
    binanceSubject.subscribePriceObserver(&priceObs);
    binanceSubject.subscribeVolumeObserver(&volumeObs);

    binanceSubject.run();

    std::this_thread::sleep_for(std::chrono::seconds(30));

    binanceSubject.stop();
    std::cout << "Stopped monitoring. Exiting program." << std::endl;

    return 0;
}
