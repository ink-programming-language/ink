// Translated from solution.cpp.

func main() -> dynamic
{
  var tot: dynamic = 0;
  var crime: dynamic = 0;
  var n: dynamic = 0;
  var x: dynamic = 0;
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
