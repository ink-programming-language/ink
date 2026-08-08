// Translated from solution.cpp.

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var inf = 1100000000;
  var n: dynamic;
  read(n);
  var pro = 1;
  var pos = 1;
  var neg = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var temp: dynamic;
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
  var x = (neg * pos);
  var val = ((n * ((n + 1))));
  write(x, cpp_char(" "), ((val / 2) - (x)), "\n");
  return 0;
}
