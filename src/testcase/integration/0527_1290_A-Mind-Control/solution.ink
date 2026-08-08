// Translated from solution.cpp.

func read(x: dynamic)
{
  x = 0;
  var c: dynamic;
  var dem = 0;
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

var MaxN = (1e6 + 1e5);

var mod = (1e9 + 7);

var n: dynamic;

var m: dynamic;

var k: dynamic;

var a = cpp_array(MaxN);

var gtln = 0;

func input()
{
  read(n, m, k);
  k = min((m - 1), k);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  gtln = 0;
  {
    var i = 0;
    while ((i <= k))
    {
      var trai = i;
      var phai = ((n - k) + i);
      var range = (n - m);
      var gtnn = INT_MAX;
      {
        var j = trai;
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

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var test = 1;
  read(test);
  while (cpp_update(test, "--"))
  {
    input();
  }
}
