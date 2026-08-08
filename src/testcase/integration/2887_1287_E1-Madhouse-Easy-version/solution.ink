// Translated from solution.cpp.

var maxn = 6e6;

var inf = 1e18;

var maxv = (2e8 + 100);

var miniinf = 1e9;

var eps = 1e-6;

var flowconst = 1e9;

func init()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
}

func main()
{
  init();
  var n: dynamic;
  read(n);
  var ans1: dynamic;
  var ans2: dynamic;
  if ((n == 1))
  {
    write("? 1 1", "\n");
    var ans: dynamic;
    read(ans);
    write("! ", ans, "\n");
    return 0;
  }
  var foo: dynamic;
  write("? 1 ", n, "\n");
  {
    var i = 0;
    while ((i < ((n * ((n + 1))) / 2)))
    {
      var x: dynamic;
      read(x);
      sort(x.begin(), x.end());
      foo[x] += 1;
      i += 1;
    }
  }
  write("? 1 ", (n - 1), "\n");
  {
    var i = 0;
    while ((i < ((((n - 1)) * (n)) / 2)))
    {
      var x: dynamic;
      read(x);
      sort(x.begin(), x.end());
      foo[x] -= 1;
      i += 1;
    }
  }
  var diff: dynamic;
  for (var ev in foo)
  {
    if ((ev.second == 1))
    {
      diff.push_back(ev.first);
    }
  }
  sort(diff.begin(), diff.end(), __cpp_lambda_1);
  var ans: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      var cnt = cpp_construct(26, 0);
      {
        var j = 0;
        while ((j < diff[i].size()))
        {
          cnt[(diff[i][j] - cpp_char("a"))] += 1;
          j += 1;
        }
      }
      {
        var j = 0;
        while ((j < ans.size()))
        {
          cnt[(ans[j] - cpp_char("a"))] -= 1;
          j += 1;
        }
      }
      {
        var j = 0;
        while ((j < 26))
        {
          if ((cnt[j] > 0))
          {
            ans += (cpp_char("a") + j);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  reverse(ans.begin(), ans.end());
  write("! ");
  write(ans, "\n");
  return 0;
}

func __cpp_lambda_1(a: dynamic, b: dynamic)
{
  return (a.size() < b.size());
}
