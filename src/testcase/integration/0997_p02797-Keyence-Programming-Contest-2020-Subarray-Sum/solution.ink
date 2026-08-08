// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  var s: dynamic;
  read(n, k, s);
  {
    var i = 0;
    while ((i < k))
    {
      write(s, cpp_char(" "));
      i += 1;
    }
  }
  if ((s != 1))
  {
    s -= 1;
  } else
  {
    s += 1;
  }
  {
    var i = k;
    while ((i < n))
    {
      write(s, cpp_char(" "));
      i += 1;
    }
  }
}
