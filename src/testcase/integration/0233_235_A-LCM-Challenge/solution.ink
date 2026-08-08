// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  read(n);
  if ((n == 1))
  {
    write("1");
  } else if ((n == 2))
  {
    write("2");
  } else if ((n & 1))
  {
    write(((n * ((n - 1))) * ((n - 2))));
  } else
  {
    if (((n % 3) == 0))
    {
      write(((((n - 1)) * ((n - 2))) * ((n - 3))));
    } else
    {
      write(((n * ((n - 1))) * ((n - 3))));
    }
  }
  return 0;
}
