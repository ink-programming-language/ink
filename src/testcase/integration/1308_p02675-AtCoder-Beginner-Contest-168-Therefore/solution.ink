// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var k = (n % 10);
  if ((((((k == 2) || (k == 4)) || (k == 5)) || (k == 7)) || (k == 9)))
  {
    write("hon");
  } else if ((k == 3))
  {
    write("bon");
  } else
  {
    write("pon");
  }
  return 0;
}
