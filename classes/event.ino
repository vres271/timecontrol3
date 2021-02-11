class Event{
  public:
    boolean fired = false;
    unsigned long payloadLong = 0;
    unsigned int payloadInt = 0;
    String payloadString = "";
    void emit(unsigned long _payloadLong=0,unsigned int _payloadInt=0) {
      payloadLong = _payloadLong;
      payloadInt = _payloadInt;
      fired = true;
    }
    void absorb() {
      fired = false;
    }
};
