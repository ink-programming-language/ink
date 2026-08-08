// Translated from solution.cpp.

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

func repl(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=(int)(a);i<(int)(b);i++)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <b");
}

func all(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++");
}

func dbg(x: dynamic)
{
  return cpp_expression("#include <bits/stdc+");
}

func mmax(x: dynamic, y: dynamic)
{
  return cpp_expression("#include");
}

func mmin(x: dynamic, y: dynamic)
{
  return cpp_expression("#include");
}

func maxch(x: dynamic, y: dynamic)
{
  return cpp_expression("#include <b");
}

func minch(x: dynamic, y: dynamic)
{
  return cpp_expression("#include <b");
}

func uni(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> using");
}

func exist(x: dynamic, y: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h>");
}

var bcnt = cpp_expression("#include <bits/stdc+");

var INF = cpp_expression("#inc");

var mod = cpp_expression("#include <");

var W: dynamic;

var H: dynamic;

var s = cpp_array(33);

var ai: dynamic;

var aj: dynamic;

var qi: dynamic;

var qj: dynamic;

var st = cpp_array(2, 33, 33, 33, 33);

var rest = cpp_array(2, 33, 33, 33, 33);

var dd = [-1, 0, 0, 1, 0, -1];

class state
{
  var qi: dynamic;
  var qj: dynamic;
  var ai: dynamic;
  var aj: dynamic;
  var t: dynamic;
}

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  while (1)
  {
    read(W, H);
    if ((W == 0))
    {
      break;
    }
    memset(st, -1, cpp_sizeof((st)));
    memset(rest, 0, cpp_sizeof((rest)));
    var que: dynamic;
    rep(sqi, H);
    rep(sqj, W);
    rep(sai, H);
    rep(saj, W);
    rep(tt, 2);
    {
      if (((s[sqi][sqj] == cpp_char("#")) || (s[sai][saj] == cpp_char("#"))))
      {
        continue;
      }
      if ((((s[sqi][sqj] == cpp_char("E")) && (!(((sqi == sai) && (sqj == saj))))) && (tt == 0)))
      {
        que.push([sqi, sqj, sai, saj, tt]);
        st[sqi][sqj][sai][saj][tt] = 1;
      } else if (((sqi == sai) && (sqj == saj)))
      {
        que.push([sqi, sqj, sai, saj, tt]);
        st[sqi][sqj][sai][saj][tt] = 0;
      } else if ((tt == 0))
      {
        cpp_statement("rep(d,5)");
        {
          var nqi = (sqi + dd[d]);
          var nqj = (sqj + dd[(d + 1)]);
          if ((((((nqi < 0) || (nqi >= H)) || (nqj < 0)) || (nqj >= W)) || (s[nqi][nqj] == cpp_char("#"))))
          {
            continue;
          }
          rest[sqi][sqj][sai][saj][tt] += 1;
        }
      } else if ((tt == 1))
      {
        cpp_statement("rep(d,5)");
        {
          var nai = (sai + dd[d]);
          var naj = (saj + dd[(d + 1)]);
          if ((((((nai < 0) || (nai >= H)) || (naj < 0)) || (naj >= W)) || (s[nai][naj] == cpp_char("#"))))
          {
            continue;
          }
          rest[sqi][sqj][sai][saj][tt] += 1;
        }
      }
    }
    while (que.size())
    {
      var c = que.front();
      que.pop();
      var crtst = st[c.qi][c.qj][c.ai][c.aj][c.t];
      if ((c.t == 1))
      {
        cpp_statement("rep(d,5)");
        {
          var nqi = (c.qi + dd[d]);
          var nqj = (c.qj + dd[(d + 1)]);
          if (((((((nqi < 0) || (nqi >= H)) || (nqj < 0)) || (nqj >= W)) || (s[nqi][nqj] == cpp_char("#"))) || (st[nqi][nqj][c.ai][c.aj][(1 - c.t)] != -1)))
          {
            continue;
          }
          if ((crtst == 1))
          {
            que.push([nqi, nqj, c.ai, c.aj, (1 - c.t)]);
            st[nqi][nqj][c.ai][c.aj][(1 - c.t)] = 1;
            continue;
          }
          rest[nqi][nqj][c.ai][c.aj][(1 - c.t)] -= 1;
          if ((rest[nqi][nqj][c.ai][c.aj][(1 - c.t)] == 0))
          {
            que.push([nqi, nqj, c.ai, c.aj, (1 - c.t)]);
            st[nqi][nqj][c.ai][c.aj][(1 - c.t)] = 0;
          }
        }
      } else if ((c.t == 0))
      {
        cpp_statement("rep(d,5)");
        {
          var nai = (c.ai + dd[d]);
          var naj = (c.aj + dd[(d + 1)]);
          if (((((((nai < 0) || (nai >= H)) || (naj < 0)) || (naj >= W)) || (s[nai][naj] == cpp_char("#"))) || (st[c.qi][c.qj][nai][naj][(1 - c.t)] != -1)))
          {
            continue;
          }
          if ((crtst == 0))
          {
            que.push([c.qi, c.qj, nai, naj, (1 - c.t)]);
            st[c.qi][c.qj][nai][naj][(1 - c.t)] = 0;
            continue;
          }
          rest[c.qi][c.qj][nai][naj][(1 - c.t)] -= 1;
          if ((rest[c.qi][c.qj][nai][naj][(1 - c.t)] == 0))
          {
            que.push([c.qi, c.qj, nai, naj, (1 - c.t)]);
            st[c.qi][c.qj][nai][naj][(1 - c.t)] = 1;
          }
        }
      }
    }
    if ((st[qi][qj][ai][aj][0] == -1))
    {
      write("Queen can not escape and Army can not catch Queen.", "\n");
    }
    if ((st[qi][qj][ai][aj][0] == 0))
    {
      write("Army can catch Queen.", "\n");
    }
    if ((st[qi][qj][ai][aj][0] == 1))
    {
      write("Queen can escape.", "\n");
    }
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
        if ((s[i][j] == cpp_char("A")))
        {
          ai = i;
          aj = j;
        }
        if ((s[i][j] == cpp_char("Q")))
        {
          qi = i;
          qj = j;
        }
      }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      read(s[i]);
    }
