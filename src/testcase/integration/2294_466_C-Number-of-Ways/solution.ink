// Translated from solution.cpp.

var v: dynamic;

func main()
{
  var n: dynamic;
  var x: dynamic;
  var sum = 0;
  var ans = 0;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(x);
      sum += x;
      v.push_back(make_pair(x, sum));
      i += 1;
    }
  }
  if (((sum % 3) == 0))
  {
    var a = (sum / 3);
    var b = (a * 2);
    x = 0;
    {
      var i = 0;
      while ((i < (n - 1)))
      {
        if ((v[i].second == b))
        {
          ans += x;
        }
        if ((v[i].second == a))
        {
          x += 1;
        }
        i += 1;
      }
    }
  }
  write(ans, "\n");
  return 0;
}
