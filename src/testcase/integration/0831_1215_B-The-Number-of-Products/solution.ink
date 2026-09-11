// Translated from solution.cpp.

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var inf: dynamic = 1100000000;
  var n: dynamic = cpp_uninitialized();
  read(n);
  var pro: dynamic = 1;
  var pos: dynamic = 1;
  var neg: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var temp: dynamic = cpp_uninitialized();
      read(temp);
      if ((temp < 0))
      {
        pro *= -1;
      }
      if ((pro < 0))
      {
        neg += 1;
      } else
      {
        pos += 1;
      }
      i += 1;
    }
  }
  var x: dynamic = (neg * pos);
  var val: dynamic = ((n * ((n + 1))));
  write(x, cpp_char(" "), ((val / 2) - (x)), "\n");
  return 0;
}
