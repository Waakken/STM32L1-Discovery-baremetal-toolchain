#include "reg_accessor.hpp"

class UART : public RegAccessor<UART>
{
public:
    UART()
        : uart1_reg(get_uart1())
        , uart2_reg(get_uart2())
        , uart3_reg(get_uart3())
        , uart4_reg(get_uart4())
        , uart5_reg(get_uart5())
    {
        init();
    };
    REG read_char(void);
    void send_char(unsigned c);
    void send_str(const char *str);
    void send_break();

private:
    void init(void);

    struct uart *uart1_reg;
    struct uart *uart2_reg;
    struct uart *uart3_reg;
    struct uart *uart4_reg;
    struct uart *uart5_reg;
};
