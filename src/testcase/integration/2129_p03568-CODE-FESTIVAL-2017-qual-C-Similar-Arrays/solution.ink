// Translated from solution.cpp.

var n: dynamic;

var arr = cpp_array(110);

func main()
{
  read(n);
  var ans = 1;
  var m = 1;
  var t: dynamic;
  {
    var j = 0;
    while ((j < n))
    {
      ans *= 3;
      read(t);
      if ((t & 1))
      {
        m *= 1;
      } else
      {
        m *= 2;
      }
      j += 1;
    }
  }
  write((ans - m));
  return 0;
}
