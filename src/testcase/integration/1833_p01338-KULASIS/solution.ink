// Translated from solution.cpp.

func REP(i: dynamic, s: dynamic, n: dynamic)
{
  cpp_macro("for(int i=s;i<n;i++)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include<b");
}

var IINF = cpp_expression("#include<");

var LEN = cpp_expression("#inclu");

var G = cpp_array(5, 7);

var dp = cpp_array(LEN, 7);

var tmp_G = cpp_array(5, 7);

func getCost(x: dynamic)
{
  return (if (((x == 3))) 80 else (if (((x == 2))) 70 else (if (((x == 1))) 60 else 0)));
}

func isValid(x: dynamic, y: dynamic)
{
  return (((((0 <= x) && (x < 5)) && (1 <= y)) && (y < 6)));
}

func push(x: dynamic, y: dynamic, cnt: dynamic)
{
  if (isValid(x, y))
  {
    (cpp_assign(tmp_G[y][x], "+=", cnt)) %= 4;
  }
  if (isValid((x + 1), y))
  {
    (cpp_assign(tmp_G[y][(x + 1)], "+=", cnt)) %= 4;
  }
  if (isValid(x, (y + 1)))
  {
    (cpp_assign(tmp_G[(y + 1)][x], "+=", cnt)) %= 4;
  }
  if (isValid((x + 1), (y + 1)))
  {
    (cpp_assign(tmp_G[(y + 1)][(x + 1)], "+=", cnt)) %= 4;
  }
}

func automatically_pusher(tmp: dynamic, y: dynamic)
{
  if ((!(((1 <= y) && (y <= 4)))))
  {
    return;
  }
  var x = 0;
  while ((x < 4))
  {
    var cnt = (tmp & ((((1 << 2)) - 1)));
    tmp >>= 2;
    push(x, y, cnt);
    x += 1;
  }
}

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(0);
  var T: dynamic;
  read(T);
  while (cpp_update(T, "--"))
  {
    cpp_statement("rep(i,7)rep(j,5)G[i][j] = -1; REP(i,1,6)rep(j,5){ cin >> G[i][j]; G[i][j]--; } int answer = 0; rep(i,7)rep(k,LEN)dp[i][k] = -1; dp[0][0] = 0; REP(i,1,6)rep(pre,LEN)");
    if ((dp[(i - 1)][pre] != -1))
    {
      rep(cur, LEN);
    }
    {
      cpp_statement("rep(j,7)rep(k,5)");
      tmp_G[j][k] = G[j][k];
      automatically_pusher(pre, (i - 1));
      automatically_pusher(cur, i);
      var cost = 0;
      rep(j, 5);
      if ((G[i][j] != -1))
      {
        cost += getCost(tmp_G[i][j]);
      }
      dp[i][cur] = max(dp[i][cur], (dp[(i - 1)][pre] + cost));
      answer = max(answer, dp[i][cur]);
    }
    write(answer, "\n");
  }
  return 0;
}
