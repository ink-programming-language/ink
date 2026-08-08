// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  var i: dynamic;
  var j: dynamic;
  var p: dynamic;
  var cnt = 2;
  var ans = 0;
  var m: dynamic;
  var l: dynamic;
  var r = 0;
  var pi = 3.1415926536;
  read(n);
  var a: dynamic;
  {
    i = 0;
    while ((i < n))
    {
      read(p);
      a.push_back(p);
      i += 1;
    }
  }
  a.push_back(0);
  sort(a.begin(), a.end());
  {
    i = n;
    while ((i > 0))
    {
      ans += (((a[i] * a[i])) - ((a[(i - 1)] * a[(i - 1)])));
      i -= 2;
    }
  }
  write((ans * pi), setprecision(9));
}
