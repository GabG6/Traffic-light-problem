class Flag {
private:
    bool state;
    bool flag;

public:
    Flag() : state(false), flag(false) {}
    /**
     * @brief Activate flag only when the sensor signal goes from 1 to 0
    */
    void setState(bool newState) {
        if (state && !newState) {
            flag = true;
        } else {
            flag = false;
        }
        state = newState;
    }

    bool isFlagSet() const {
        return flag;
    }

    void resetFlag() {
        flag = false;
    }
};