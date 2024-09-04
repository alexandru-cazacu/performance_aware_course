static void write_to_all_bytes(RepetitionTester* tester, ReadParams* params) {
    while (tester_is_testing(tester)) {
        String destBuffer = params->dest;
        handle_allocation(params, &destBuffer);
        
        tester_begin_time(tester);
        for (u64 i = 0; i < destBuffer.count; ++i) {
            destBuffer.data[i] = (u8)i;
        }
        tester_end_time(tester);
        
        count_bytes(tester, destBuffer.count);
        
        handle_deallocation(params, &destBuffer);
    }
}