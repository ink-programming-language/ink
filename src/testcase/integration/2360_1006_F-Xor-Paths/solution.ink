// Translated from solution.cpp.

var eps = 0.00000001;

var MOD = (1e9 + 7);

var PI = 3.141592653589793238463;

var ans: dynamic;

var k: dynamic;

var n: dynamic;

var m: dynamic;

var a = cpp_array(25, 25);

var cnt: dynamic;

var half: dynamic;

func isOK(x: dynamic, y: dynamic)
{
  if (((((x >= 1) && (x <= n)) && (y >= 1)) && (y <= m)))
  {
    return true;
  }
  return false;
}

func dfs(x: dynamic, y: dynamic, res: dynamic)
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

func back_dfs(x: dynamic, y: dynamic, res: dynamic)
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

func main()
{
  fflush(stdin);
  write(fixed);
  cout.precision(18);
  ios_base.sync_with_stdio(false);
  var i: dynamic;
  var j: dynamic;
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
