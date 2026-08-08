// Translated from solution.cpp.

func toint(s: dynamic)
{
  var ss: dynamic;
  (ss << s);
  var x: dynamic;
  (ss >> x);
  return x;
}

func tostring(number: dynamic)
{
  var ss: dynamic;
  (ss << number);
  return ss.str();
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic;
  read(t);
  {
    var tt = 0;
    while ((tt < t))
    {
      var n: dynamic;
      var a = 0;
      var r = 0;
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
