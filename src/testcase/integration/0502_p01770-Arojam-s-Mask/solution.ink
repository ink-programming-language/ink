// Translated from solution.cpp.

func r(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<n;i++)");
}

var dp = cpp_array((1 << 5), 111, 111);

var n: dynamic;

var m: dynamic;

var e: dynamic;

var s: dynamic;

var t: dynamic;

var r: dynamic;

var v = cpp_array(100000);

func main()
{
  cpp_statement("r(i,111)r(j,1<<5)r(k,111)");
  dp[i][k][j] = 1e9;
  read(n, m, e, s, t, r);
  var M: dynamic;
  var state = 0;
  var q: dynamic;
  q.push(P2(P(0, s), P(r, state)));
  dp[s][r][state] = 0;
  while ((!q.empty()))
  {
    var PP = q.top();
    q.pop();
    var now = PP.first.second;
    var cost = PP.first.first;
    var R = PP.second.first;
    var S = PP.second.second;
    if ((dp[now][R][S] < cost))
    {
      continue;
    }
    if (R)
    {
      r(i, v[now].size());
      {
        var nex = v[now][i].first;
        var flag = v[now][i].second;
        var f = (flag % 100);
        var nR = (R - 1);
        var nS = S;
        if (M.count(nex))
        {
          nS = ((nS | ((1 << M[nex]))));
        }
        if ((flag == -1))
        {
          if ((dp[nex][nR][nS] <= (cost + 1)))
          {
            continue;
          }
          dp[nex][nR][nS] = (cost + 1);
          q.push(P2(P((cost + 1), nex), P(nR, nS)));
        } else
        {
          if ((!((S & ((1 << f))))))
          {
            continue;
          }
          if ((dp[nex][nR][nS] <= (cost + 1)))
          {
            continue;
          }
          dp[nex][nR][nS] = (cost + 1);
          q.push(P2(P((cost + 1), nex), P(nR, nS)));
        }
      }
    }
    if ((dp[s][r][S] <= (cost + 1)))
    {
      continue;
    }
    dp[s][r][S] = (cost + 1);
    q.push(P2(P((cost + 1), s), P(r, S)));
  }
  var ans = 1e9;
  r(i, 101);
  r(j, ((1 << 5)));
  {
    ans = min(ans, dp[t][i][j]);
  }
  if ((ans == 1e9))
  {
    write(-1, "\n");
  } else
  {
    write(ans, "\n");
  }
}

func r(argument_0: dynamic, argument_1: dynamic)
{
    var a: dynamic;
    var b: dynamic;
    read(a, b);
    v[a].push_back(P(b, -1));
    v[b].push_back(P(a, -1));
  }

func r(argument_0: dynamic, argument_1: dynamic)
{
    var a: dynamic;
    var b: dynamic;
    var c: dynamic;
    read(a, b, c);
    v[a].push_back(P(b, i));
    v[b].push_back(P(a, i));
    M[c] = i;
    if ((c == s))
    {
      state |= ((1 << i));
    }
  }
