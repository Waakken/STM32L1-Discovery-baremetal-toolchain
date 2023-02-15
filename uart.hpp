#include "reg_accessor.hpp"

class UART : private RegAccessor
{
public:
    UART()
        : uart1_reg(get_uart1())
        , uart2_reg(get_uart2())
        , uart3_reg(get_uart3())
        , uart4_reg(get_uart4())
        , uart5_reg(get_uart5()){};
    void init(void);
    char read_char(void);

private:
    struct uart *uart1_reg;
    struct uart *uart2_reg;
    struct uart *uart3_reg;
    struct uart *uart4_reg;
    struct uart *uart5_reg;
};
