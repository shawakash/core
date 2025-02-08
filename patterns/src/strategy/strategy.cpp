#include <iostream>

/**
    Use strategy pattern only when there is a need to reuse code in child classes of a base class
    else consider implementing it inside the child class itself
*/

// Interface for quack behaviour for cross class function exchange
class IQuackBehaviour {
  public:
    virtual void quack() = 0;
    virtual ~IQuackBehaviour() {}
};

class SimpleQuack : public IQuackBehaviour {
  public:
    void quack() override {
      std::cout << "Quack!" << std::endl;
    }
};

class NoQuack : public IQuackBehaviour {
  public:
    void quack() override {
        std::cout << "<< Silence >>" << std::endl;
    }
};

// Interface for fly behaviour for cross class function exchange
class IFlyBehaviour {
    public:
        virtual void fly() = 0;
        virtual ~IFlyBehaviour() {}
};

class NormalFly : public IFlyBehaviour {
    public:
        void fly() override {
            std::cout << "This is a normal fly!" << std::endl;
        }
};

class HopFly : public IFlyBehaviour {
    public:
        void fly() override {
            std::cout << "This is Hopping fly!!" << std::endl;
        }
};

// Base class for Duck
class Duck {
    protected:
    // Composition : Duck has a quack and Fly behaviour
        IQuackBehaviour* quackBehaviour;
        IFlyBehaviour* flyBehaviour;
    public:
        Duck(IQuackBehaviour* qb, IFlyBehaviour* fb) : quackBehaviour(qb), flyBehaviour(fb) {}

        virtual void display() = 0;

        virtual void fly() {
            if (flyBehaviour) {
                flyBehaviour->fly();
            }
        }

        void quack() {
            if (quackBehaviour) {
                quackBehaviour->quack();
            }
        }

        void setQuackBehaviour(IQuackBehaviour* qb) {
            delete quackBehaviour;
            quackBehaviour = qb;
        }

        void setFlyBehaviour(IFlyBehaviour* fb) {
            delete flyBehaviour;
            flyBehaviour = fb;
        }

        virtual ~Duck() {
            delete quackBehaviour;
            delete flyBehaviour;
        }
};

class WildDuck : public Duck {
    public:
        WildDuck(IQuackBehaviour* qb, IFlyBehaviour* fb) : Duck(qb, fb) {}

        void display() override {
            std::cout << "Hi, I am a Wild Duck!" << std::endl;
        }
};

class CityDuck : public Duck {
    public:
        CityDuck(IQuackBehaviour* qb, IFlyBehaviour* fb) : Duck(qb, fb) {}

        void display() override {
            std::cout << "Hello, I am a City Duck!" << std::endl;
        }
};

int main() {

    Duck* cityDuck = new CityDuck(new SimpleQuack(), new NormalFly());
    Duck* wildDuck = new WildDuck(new NoQuack(), new HopFly());

    std::cout << "\n\n----- Presenting City Duck -----\n";
    cityDuck->display();
    cityDuck->fly();
    cityDuck->quack();
    std::cout << "\n----- Changing Quack Behaviour -----\n";
    cityDuck->setQuackBehaviour(new NoQuack());
    cityDuck->quack();

    std::cout << "\n\n----- Now Presenting Wild Duck -----\n";
    wildDuck->display();
    wildDuck->fly();
    wildDuck->quack();
    std::cout << "\n----- Changing Quack Behaviour -----\n";
    wildDuck->setQuackBehaviour(new SimpleQuack());
    wildDuck->quack();


    return 0;
}
