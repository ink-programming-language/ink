// Translated from solution.cpp.

func r(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<n;i++)");
}

var dp: dynamic = cpp_array((1 << 5), 111, 111);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var e: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var v: dynamic = cpp_array(100000);

func main() -> dynamic
{
  cpp_statement("r(i,111)r(j,1<<5)r(k,111)");
  dp[i][k][j] = 1e9;
  read(n, m, e, s, t, r);
  var M: dynamic = cpp_uninitialized();
  var state: dynamic = 0;
  var q: dynamic = cpp_uninitialized();
  q.push(P2(P(0, s), P(r, state)));
  dp[s][r][state] = 0;
  while ((!q.empty()))
  {
    var PP: dynamic = q.top();
    q.pop();
    var now: dynamic = PP.first.second;
    var cost: dynamic = PP.first.first;
    var R: dynamic = PP.second.first;
    var S: dynamic = PP.second.second;
    if ((dp[now][R][S] < cost))
    {
      continue;
    }
    if (R)
    {
      r(i, v[now].size());
      {
        var nex: dynamic = v[now][i].first;
        var flag: dynamic = v[now][i].second;
        var f: dynamic = (flag % 100);
        var nR: dynamic = (R - 1);
        var nS: dynamic = S;
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
  var ans: dynamic = 1e9;
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

func r(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    read(a, b);
    v[a].push_back(P(b, -1));
    v[b].push_back(P(a, -1));
  }

func r(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    var c: dynamic = cpp_uninitialized();
    read(a, b, c);
    v[a].push_back(P(b, i));
    v[b].push_back(P(a, i));
    M[c] = i;
    if ((c == s))
    {
      state |= ((1 << i));
    }
  }
