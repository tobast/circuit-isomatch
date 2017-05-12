#include <stdio.h>
#include <c_api/isomatch.h>

int main() {
    // Let's hardcode a circuit \o/

    circuit_handle g_root = build_group("root");
    build_group_add_input(g_root, "p1", "p1");
    build_group_add_input(g_root, "p2", "p2");
    build_group_add_output(g_root, "out", "out");

    circuit_handle g_sub = build_group("sub");
    build_group_add_input(g_sub, "inp", "p1");
    build_group_add_child(g_root, g_sub);
    build_group_add_output(g_sub, "out", "sub_out");

    circuit_handle c_sub = build_comb(g_sub);
    build_comb_add_input(c_sub, "inp");
    expr_handle c_sub_expr = build_expr_binop(BXor,
            build_expr_unop(UNot,
                build_expr_var(0)),
            build_expr_var(0));
    build_comb_add_output(c_sub, "out", c_sub_expr);

    build_delay(g_root, "sub_out", "delay_out");
    build_tristate(g_root, "sub_out", "out", "delay_out");
    circuit_handle c_delay_not = build_comb(g_root);
    build_comb_add_input(c_delay_not, "delay_out");
    build_comb_add_output(c_delay_not, "delay_out_not",
            build_expr_unop(UNot, build_expr_var(0)));
    build_tristate(g_root, "p2", "out", "delay_out_not");


    freeze_circuit(g_root);
    printf("%lX\n", sign(g_root));

    free_circuit(g_root);

    return 0;
}

