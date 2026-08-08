// Translated from solution.cpp.

func main()
{
  var tot = 0;
  var crime = 0;
  var n = 0;
  var x = 0;
  read(n);
  while (cpp_update(n, "--"))
  {
    read(x);
    tot += x;
    if ((tot < 0))
    {
      crime += 1;
      tot += 1;
    }
  }
  write(crime, "\n");
  return 0;
}
