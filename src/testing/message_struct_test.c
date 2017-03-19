/* #include "message_struct_test.h" */
#include "../utils/struct_message.h"
#include "../msgserv/message.h"
#include "greatest.h"


TEST test_parse_not_full(void) {
    char output[4098];
    bzero(output, 4098);

    char to_parse[4098] = "SMESSAGES\n"
        "0;Lorem ipsum dolor sit amet, consectetur adipiscing elit\n"
        "1;Duo Reges: constructio interrete.\n"
        "2;Atqui pugnantibus et contrariis studiis consiliisque semper utens nihil quieti videre, nihil tranquilli potest.\n"
        "3;Estne, quaeso, inquam, sitienti in bibendo voluptas? Facit igitur Lucius noster prudenter\n"
        "4;Atqui reperies, inquit, in hoc quidem pertinacem; De ingenio eius in his disputationibus, non de moribus quaeritur. Atque ab his initiis profecti omnium virtutum et originem et progressionem persecuti sunt\n";
        // Msg 4 overfills c.c 206

    const char *expected = "LC: 0 Message: Lorem ipsum dolor sit amet, consectetur adipiscing elit\n"
          "LC: 1 Message: Duo Reges: constructio interrete.\n"
          "LC: 2 Message: Atqui pugnantibus et contrariis studiis consiliisque semper utens nihil quieti videre, nihil tranquilli potest.\n"
          "LC: 3 Message: Estne, quaeso, inquam, sitienti in bibendo voluptas? Facit igitur Lucius noster prudenter\n"
          "LC: 4 Message: Atqui reperies, inquit, in hoc quidem pertinacem; De ingenio eius in his disputationibus, non de moribus quaeritur. Atque ab his initiis pro\n";

    matrix this = create_matrix(8);
    parse_messages(to_parse, this);
    freopen("/dev/null", "a", stdout);
    setbuf(stdout, output);
    print_matrix(this, print_message_plain);
    freopen ("/dev/tty", "a", stdout);

    ASSERT_STR_EQ(expected, output);

    free_matrix(this, free_message);
    PASS();
}

TEST teacher_example_douro(void) {
    g_lc = 0;
    char output[4098];
    bzero(output, 4098);
    matrix msg_matrix = create_matrix(20);

    // Guadiana Comms
    ASSERT_EQ(2, handle_publish(msg_matrix, "sabem qual o programa das JEEC?"));

    ASSERT_STR_EQ("sabem qual o programa das JEEC?\n", get_first_n_messages(msg_matrix, 10));

    ASSERT_EQ(2, handle_publish(msg_matrix, "vê em jeec.tecnico.ulisboa.pt"));
    ASSERT_STR_EQ("sabem qual o programa das JEEC?\n"
            "vê em jeec.tecnico.ulisboa.pt"
            , get_first_n_messages(msg_matrix, 10));

    ASSERT_EQ(2, g_lc);

    // Lima Comms

    PASS();
}

GREATEST_SUITE(msg_struct) {
    RUN_TEST(test_parse_not_full);
    /* RUN_TEST(teacher_example_douro); */
}

