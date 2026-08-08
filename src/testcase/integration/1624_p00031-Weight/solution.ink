// Translated from solution.cpp.

func main(argument_0: dynamic)
{
  var n: dynamic;
  var ans: dynamic;
  while ((cin >> n))
  {
    {
      var i = 0;
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
