// Translated from solution.cpp.

var eps: dynamic = 0.00000001;

var MOD: dynamic = (1e9 + 7);

var PI: dynamic = 3.141592653589793238463;

var ans: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(25, 25);

var cnt: dynamic = cpp_uninitialized();

var half: dynamic = cpp_uninitialized();

func isOK(x: dynamic, y: dynamic) -> dynamic
{
  if (((((x >= 1) && (x <= n)) && (y >= 1)) && (y <= m)))
  {
    return true;
  }
  return false;
}

func dfs(x: dynamic, y: dynamic, res: dynamic) -> dynamic
{
  res ^= a[x][y];
  if (((x + y) == half))
  {
    cnt[[[x, y], res]] += 1;
    return;
  }
  if (isOK((x + 1), y))
  {
    dfs((x + 1), y, res);
  }
  if (isOK(x, (y + 1)))
  {
    dfs(x, (y + 1), res);
  }
}

func back_dfs(x: dynamic, y: dynamic, res: dynamic) -> dynamic
{
  if (((x + y) == half))
  {
    ans += cnt[[[x, y], ((res ^ k))]];
    return;
  }
  res ^= a[x][y];
  if (isOK((x - 1), y))
  {
    back_dfs((x - 1), y, res);
  }
  if (isOK(x, (y - 1)))
  {
    back_dfs(x, (y - 1), res);
  }
}

func main() -> dynamic
{
  fflush(stdin);
  write(fixed);
  cout.precision(18);
  ios_base.sync_with_stdio(false);
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  read(n, m, k);
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 1;
        while ((j <= m))
        {
          read(a[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  half = ((((m + n)) / 2) + 1);
  dfs(1, 1, 0);
  back_dfs(n, m, 0);
  write(ans);
}
