#include <stdio.h>
#include <c_api/isomatch.h>

int main() {
    // Let's hardcode a circuit \o/

    circuit_handle g_root = build_group("root");
    build_group_add_input(g_root, "p1", "p1");
    build_group_add_input(g_root, "p2", "p2");
    build_group_add_input(g_root, "p3", "p3");
    build_group_add_output(g_root, "out", "out");
    build_group_add_output(g_root, "mux1out", "mux1out");

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

    circuit_handle c_mux1_not = build_comb(g_root);
    build_comb_add_input(c_mux1_not, "p1");
    build_comb_add_output(c_mux1_not, "np1",
            build_expr_unop(UNot, build_expr_var(0)));
    build_tristate(g_root, "p2", "mux1out", "p1");
    build_tristate(g_root, "p3", "mux1out", "np1");

    freeze_circuit(g_root);
    printf("%lX\n", sign(g_root));

    circuit_handle g_needle = build_group("needle_mux");
    build_group_add_input(g_needle, "a", "a");
    build_group_add_input(g_needle, "b", "b");
    build_group_add_input(g_needle, "sel", "sel");
    build_group_add_output(g_needle, "out", "out");
    circuit_handle c_needle_not = build_comb(g_needle);
    build_comb_add_input(c_needle_not, "sel");
    build_comb_add_output(c_needle_not, "nsel",
            build_expr_unop(UNot, build_expr_var(0)));
    build_tristate(g_needle, "a", "out", "sel");
    build_tristate(g_needle, "b", "out", "nsel");

    freeze_circuit(g_needle);

    match_results* res = subcircuit_find(g_needle, g_root);
    int matches = 0;
    while(res != NULL) {
        res = res->next;
        ++matches;
    }
    printf("%d MUX\n", matches);

    free_match_results(res);
    free_circuit(g_root);
    free_circuit(g_needle);

    return 0;
}

