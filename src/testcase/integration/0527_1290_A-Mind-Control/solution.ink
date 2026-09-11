// Translated from solution.cpp.

func read(x: dynamic) -> dynamic
{
  x = 0;
  var c: dynamic = cpp_uninitialized();
  var dem: dynamic = 0;
  {
    c = getchar();
    while (((c < cpp_char("0")) || (c > cpp_char("9"))))
    {
      dem += 1;
      if ((dem == 100))
      {
        return;
      }
      c = getchar();
    }
  }
  {
    while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
    {
      x = (((x * 10) + c) - cpp_char("0"));
      c = getchar();
    }
  }
}

var MaxN: dynamic = (1e6 + 1e5);

var mod: dynamic = (1e9 + 7);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(MaxN);

var gtln: dynamic = 0;

func input() -> dynamic
{
  read(n, m, k);
  k = min((m - 1), k);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  gtln = 0;
  {
    var i: dynamic = 0;
    while ((i <= k))
    {
      var trai: dynamic = i;
      var phai: dynamic = ((n - k) + i);
      var range: dynamic = (n - m);
      var gtnn: dynamic = INT_MAX;
      {
        var j: dynamic = trai;
        while ((j < (phai - range)))
        {
          gtnn = min(gtnn, max(a[j], a[(j + range)]));
          j += 1;
        }
      }
      gtln = max(gtnn, gtln);
      i += 1;
    }
  }
  write(gtln, cpp_char("\n"));
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var test: dynamic = 1;
  read(test);
  while (cpp_update(test, "--"))
  {
    input();
  }
}
