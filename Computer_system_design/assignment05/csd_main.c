
void csd_main(unsigned char *hour, unsigned char *min, unsigned char *sec, unsigned char *flag)   // getting hour, min, sec, flag in string
{
   if(flag[0] == '1')
   {
	   flag[0] = '0';
	   return ;
   }

	int s = (sec[0] - '0') * 10 + (sec[1] - '0');         // string to integer(second)
   int m = (min[0] - '0') * 10 + (min[1] - '0');         // string to integer(minute)
   int h = (hour[0] - '0') * 10 + (hour[1] - '0');         // string to integer(hour)

   s += 1;               // add 1 second to clock
   if(s == 60)            // if current second is 60
   {
      s = 0;            // set second to 0
      m += 1;            // add 1 minute
   }

   if(m == 60)            // if current minute is 60
   {
      m = 0;            // set minute to 0
      h += 1;            // add 1 hour
   }

   if(h == 24)            // if current second is 60
   {
      h = 0;            // set hour to 0 (initialize to 00:00:00)
   }

   sec[0] = (s / 10) + '0';      // sec[0] => tens of second
   sec[1] = (s % 10) + '0';      // sec[1] => ones of second

   min[0] = (m / 10) + '0';      // min[0] => tens of second
   min[1] = (m % 10) + '0';      // min[1] => ones of second

   hour[0] = (h / 10) + '0';      // hour[0] => tens of second
   hour[1] = (h % 10) + '0';      // hour[1] => ones of second
}
