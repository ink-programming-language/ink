// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var arr: dynamic = cpp_array(110);

func main() -> dynamic
{
  read(n);
  var ans: dynamic = 1;
  var m: dynamic = 1;
  var t: dynamic = cpp_uninitialized();
  {
    var j: dynamic = 0;
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
