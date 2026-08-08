// Translated from solution.cpp.

var MAXN = (1e5 + 20);

var n: dynamic;

var vec = cpp_array(MAXN);

var p = cpp_array(MAXN);

var ans = cpp_array(MAXN);

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(vec[i]);
      i += 1;
    }
  }
  sort(vec, (vec + n), greater());
  {
    var i = (n - 1);
    while ((i >= 0))
    {
      p[i] = ((p[(i + 1)] + vec[i]));
      i -= 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      ans[i] = p[1];
      var s = i;
      {
        var j = (i + 1);
        while ((j < n))
        {
          ans[i] += p[j];
          s *= i;
          j += s;
        }
      }
      i += 1;
    }
  }
  var q: dynamic;
  read(q);
  while (cpp_update(q, "--"))
  {
    var k: dynamic;
    read(k);
    k = min(k, (n - 1));
    write(ans[k], " ");
  }
  write("\n");
  return 0;
}
