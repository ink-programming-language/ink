// Translated from solution.cpp.

var MAXN: dynamic = (1e5 + 20);

var n: dynamic = cpp_uninitialized();

var vec: dynamic = cpp_array(MAXN);

var p: dynamic = cpp_array(MAXN);

var ans: dynamic = cpp_array(MAXN);

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(vec[i]);
      i += 1;
    }
  }
  sort(vec, (vec + n), greater());
  {
    var i: dynamic = (n - 1);
    while ((i >= 0))
    {
      p[i] = ((p[(i + 1)] + vec[i]));
      i -= 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      ans[i] = p[1];
      var s: dynamic = i;
      {
        var j: dynamic = (i + 1);
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
  var q: dynamic = cpp_uninitialized();
  read(q);
  while (cpp_update(q, "--"))
  {
    var k: dynamic = cpp_uninitialized();
    read(k);
    k = min(k, (n - 1));
    write(ans[k], " ");
  }
  write("\n");
  return 0;
}
