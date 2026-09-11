// Translated from solution.cpp.

var maxn: dynamic = 6e6;

var inf: dynamic = 1e18;

var maxv: dynamic = (2e8 + 100);

var miniinf: dynamic = 1e9;

var eps: dynamic = 1e-6;

var flowconst: dynamic = 1e9;

func init() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
}

func main() -> dynamic
{
  init();
  var n: dynamic = cpp_uninitialized();
  read(n);
  var ans1: dynamic = cpp_uninitialized();
  var ans2: dynamic = cpp_uninitialized();
  if ((n == 1))
  {
    write("? 1 1", "\n");
    var ans: dynamic = cpp_uninitialized();
    read(ans);
    write("! ", ans, "\n");
    return 0;
  }
  var foo: dynamic = cpp_uninitialized();
  write("? 1 ", n, "\n");
  {
    var i: dynamic = 0;
    while ((i < ((n * ((n + 1))) / 2)))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      sort(x.begin(), x.end());
      foo[x] += 1;
      i += 1;
    }
  }
  write("? 1 ", (n - 1), "\n");
  {
    var i: dynamic = 0;
    while ((i < ((((n - 1)) * (n)) / 2)))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      sort(x.begin(), x.end());
      foo[x] -= 1;
      i += 1;
    }
  }
  var diff: dynamic = cpp_uninitialized();
  for (var ev: dynamic in foo)
  {
    if ((ev.second == 1))
    {
      diff.push_back(ev.first);
    }
  }
  sort(diff.begin(), diff.end(), __cpp_lambda_1);
  var ans: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var cnt: dynamic = cpp_construct(26, 0);
      {
        var j: dynamic = 0;
        while ((j < diff[i].size()))
        {
          cnt[(diff[i][j] - cpp_char("a"))] += 1;
          j += 1;
        }
      }
      {
        var j: dynamic = 0;
        while ((j < ans.size()))
        {
          cnt[(ans[j] - cpp_char("a"))] -= 1;
          j += 1;
        }
      }
      {
        var j: dynamic = 0;
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

func __cpp_lambda_1(a: dynamic, b: dynamic) -> dynamic
{
  return (a.size() < b.size());
}
