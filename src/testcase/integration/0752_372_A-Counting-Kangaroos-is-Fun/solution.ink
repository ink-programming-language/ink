// Translated from solution.cpp.

var v: dynamic;

func main()
{
  var n: dynamic;
  var c = 0;
  read(n);
  var x: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      read(x);
      v.push_back(x);
      i += 1;
    }
  }
  sort(v.begin(), v.end());
  var r = (n - 1);
  {
    var i = ((n / 2) - 1);
    while ((i >= 0))
    {
      if (((2 * v[i]) <= v[r]))
      {
        c += 1;
        r -= 1;
      }
      i -= 1;
    }
  }
  write((n - c));
  return 0;
}
