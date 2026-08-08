// Translated from solution.cpp.

func REP(i: dynamic, b: dynamic, n: dynamic)
{
  cpp_macro("for(int i=b;i<n;i++)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include<i");
}

var inf = 100;

var N = 5;

var dy = [1, 1, -1, -1];

var dx = [0, 1, 0, -1];

var wdy = [-1, 1];

var edge = cpp_array(N, N, 66605);

var wdcost = cpp_array(66605, (N + 1));

class st
{
  var mat: dynamic = cpp_array(N, N);
  func st()
  {
      cpp_statement("rep(i,N)rep(j,N)mat[i][j] = 0; rep(i,N)");
      mat[i][i] = (N - i);
      mat[0][0] = (N - 1);
    }
  func operator_less(a: dynamic)
  {
      cpp_statement("rep(i,N)rep(j,N)");
      if ((mat[i][j] != a.mat[i][j]))
      {
        return (mat[i][j] < a.mat[i][j]);
      }
      return false;
    }
}

func getst(now: dynamic, M: dynamic)
{
  var index = M.size();
  if ((M.count(now) == 0))
  {
    M[now] = index;
  }
  return M[now];
}

var NUM: dynamic;

func makeall()
{
  var M: dynamic;
  var Q: dynamic;
  var ini: dynamic;
  Q.push(ini);
  M.insert(make_pair(ini, 0));
  while ((!Q.empty()))
  {
    var now = Q.front();
    Q.pop();
    var nownum = getst(now, NUM);
    var tc = M[now];
    wdcost[N][nownum] = tc;
  }
}

var row = cpp_array(15);

var col = cpp_array(15);

func precalcWD()
{
  var ori = [1, 3, 6, 10, 15, 2, 5, 9, 14, 0, 4, 8, 13, 0, 0, 7, 12, 0, 0, 0, 11, 0, 0, 0, 0];
}

func getWD(n: dynamic, cpy: dynamic)
{
  if ((n != 5))
  {
    return make_pair(0, 0);
  }
  var r: dynamic;
  var c: dynamic;
  var inp = [0];
  var matr = [];
  var matc = [];
  rep(i, N);
  rep(j, N).mat[i][j] = matr[i][j];
  rep(i, N);
  rep(j, N).mat[i][j] = matc[i][j];
  return make_pair(NUM[r], NUM[c]);
}

func initWD()
{
  rep(i, 66605);
  rep(j, N);
  rep(k, N)[i][j][k] = -1;
  makeall();
  precalcWD();
}

var mcost = cpp_array(N, N, (N * N));

func precalc(n: dynamic)
{
  var pos = cpp_array(n, n);
  var p = 0;
  while (true)
  {
    var isupdate = false;
    if ((!isupdate))
    {
      break;
    }
  }
}

var in_cpp = cpp_array(N, N);

func geth(n: dynamic)
{
  cpp_statement("rep(i,n)rep(j,i+1)");
  mcost[0][i][j] = 0;
  var ret = 0;
  return ret;
}

var ans: dynamic;

func solve(n: dynamic, cnt: dynamic, lim: dynamic, py: dynamic, px: dynamic, prev: dynamic, h: dynamic, wdr: dynamic, wdc: dynamic, y: dynamic, x: dynamic)
{
  if ((h == 0))
  {
    ans = min(ans, cnt);
    return true;
  }
  if (((cnt + h) > lim))
  {
    return false;
  }
  if (((n == 5) && (((cnt + wdcost[5][wdr]) + wdcost[5][wdc]) > lim)))
  {
    return false;
  }
  rep(k, 4);
  {
    var ney = (y + dy[k]);
    var nex = (x + dx[k]);
    if (((((ney == -1) || (nex == -1)) || (ney == n)) || (ney < nex)))
    {
      continue;
    }
    if ((in_cpp[ney][nex] == prev))
    {
      continue;
    }
    var nexth = (h + ((mcost[(in_cpp[ney][nex] - 1)][y][x] - mcost[(in_cpp[ney][nex] - 1)][ney][nex])));
    var nextwdr = wdr;
    var nextwdc = wdc;
    if ((n == 5))
    {
      if (((k == 0) || (k == 2)))
      {
        assert((nex == x));
        var base = x;
        nextwdr = edge[wdr][(ney - base)][row[in_cpp[ney][nex]]];
      } else
      {
        nextwdc = edge[wdc][nex][col[in_cpp[ney][nex]]];
      }
    }
    swap(in_cpp[y][x], in_cpp[ney][nex]);
    var ret: dynamic;
    ret = solve(n, (cnt + 1), lim, -1, -1, in_cpp[y][x], nexth, nextwdr, nextwdc, ney, nex);
    swap(in_cpp[y][x], in_cpp[ney][nex]);
    if (ret)
    {
      return true;
    }
  }
  return false;
}

func main()
{
  initWD();
  var n: dynamic;
  var tc = 1;
  while (((cin >> n) && n))
  {
    ans = inf;
    precalc(n);
    var sy: dynamic;
    var sx: dynamic;
    rep(i, n);
    rep(j, (i + 1));
    if ((in_cpp[i][j] == 1))
    {
      sy = i;
      sx = j;
    }
    var mod = mcost[0][sy][sx];
    var h = geth(n);
    var beg = 0;
    var wd = getWD(n, in_cpp);
    var wdr = wd.first;
    var wdc = wd.second;
    if (((mod % 2) != 0))
    {
      beg += 1;
    }
    {
      var i = beg;
      while (true)
      {
        ans = inf;
        var ret = solve(n, 0, i, -1, -1, -1, h, wdr, wdc, sy, sx);
        if (ret)
        {
          break;
        }
        i += 2;
      }
    }
    write("Case ", cpp_update(tc, "++"), ": ", ans, "\n");
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
        cpp_statement("rep(k,2)");
        {
          var ney = (i + wdy[k]);
          if (((ney == -1) || (ney == N)))
          {
            continue;
          }
          if ((now.mat[ney][j] == 0))
          {
            continue;
          }
          var next = now;
          next.mat[i][j] += 1;
          next.mat[ney][j] -= 1;
          var nextnum = getst(next, NUM);
          edge[nownum][ney][j] = nextnum;
          if ((M.count(next) == 0))
          {
            M.insert(make_pair(next, (tc + 1)));
            Q.push(next);
          }
        }
      }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      if ((accumulate(now.mat[i], now.mat[(i + 1)], 0) == (N - i)))
      {
        continue;
      }
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      if (((ori[i][j] == 1) || (ori[i][j] == 0)))
      {
        continue;
      }
      row[ori[i][j]] = i;
      col[ori[i][j]] = j;
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      if ((cpy[i][j] == 0))
      {
        continue;
      }
      inp[cpp_update(p, "++")][j] = cpy[i][j];
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var p = 0;
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      if (((inp[i][j] == 0) || (inp[i][j] == 1)))
      {
        continue;
      }
      matr[i][row[inp[i][j]]] += 1;
      matc[j][col[inp[i][j]]] += 1;
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    cpp_statement("rep(j,i+1)");
    {
      cpp_statement("rep(k,n)rep(l,k+1)");
      mcost[p][k][l] = inf;
      mcost[p][i][j] = 0;
      pos[i][j] = p;
      p += 1;
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
          cpp_statement("rep(j,i+1)");
          {
            cpp_statement("rep(k,4)");
            {
              var ney = (i + dy[k]);
              var nex = (j + dx[k]);
              if (((((ney == -1) || (nex == -1)) || (ney == n)) || (ney < nex)))
              {
                continue;
              }
              if (((ney == i) && (nex == (i + 1))))
              {
                continue;
              }
              if ((mcost[now][ney][nex] > (mcost[now][i][j] + 1)))
              {
                isupdate = true;
                mcost[now][ney][nex] = (mcost[now][i][j] + 1);
              }
              if ((mcost[now][i][j] > (mcost[now][ney][nex] + 1)))
              {
                isupdate = true;
                mcost[now][i][j] = (mcost[now][ney][nex] + 1);
              }
            }
          }
        }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      rep(jj, (ii + 1));
      {
        var now = pos[ii][jj];
      }
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    cpp_statement("rep(j,i+1)");
    {
      ret += mcost[(in_cpp[i][j] - 1)][i][j];
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      cpp_statement("rep(j,i+1)");
      {
        read(in_cpp[i][j]);
      }
    }
