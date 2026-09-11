// Translated from solution.cpp.

func toint(s: dynamic) -> dynamic
{
  var ss: dynamic = cpp_uninitialized();
  (ss << s);
  var x: dynamic = cpp_uninitialized();
  (ss >> x);
  return x;
}

func tostring(number: dynamic) -> dynamic
{
  var ss: dynamic = cpp_uninitialized();
  (ss << number);
  return ss.str();
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = cpp_uninitialized();
  read(t);
  {
    var tt: dynamic = 0;
    while ((tt < t))
    {
      var n: dynamic = cpp_uninitialized();
      var a: dynamic = 0;
      var r: dynamic = 0;
      read(n);
      while ((n > 9))
      {
        a += (n - (n % 10));
        r = ((n / 10) + (n % 10));
        n = r;
      }
      write((a + n), "\n");
      tt += 1;
    }
  }
  return 0;
}
