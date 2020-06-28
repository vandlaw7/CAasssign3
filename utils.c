printf("ocheck: ");
printf("%d", old_data);
printf("end ");

printf("ncheck:");
printf("%d", new_data);
printf("end ");
cache_write(addr, new_data);
