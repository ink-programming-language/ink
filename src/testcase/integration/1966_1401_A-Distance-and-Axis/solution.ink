// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var k: dynamic;
  var n: dynamic;
  var t: dynamic;
  read(t);
  {
    while (cpp_update(t, "--"))
    {
      read(n, k);
      if ((n < k))
      {
        write((k - n), cpp_char("\n"));
      } else if (((n % 2) == (k % 2)))
      {
        write(0, cpp_char("\n"));
      } else
      {
        write(1, cpp_char("\n"));
      }
    }
  }
  return 0;
}
