// Translated from solution.cpp.

func main(argument_0: dynamic) -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
  while ((cin >> n))
  {
    {
      var i: dynamic = 0;
      while ((i < 10))
      {
        if ((n & 1))
        {
          ans = (1 << i);
          write(ans);
          if ((n >> 1))
          {
            write(cpp_char(" "));
          }
        }
        n >>= 1;
        i += 1;
      }
    }
    write("\n");
  }
}
