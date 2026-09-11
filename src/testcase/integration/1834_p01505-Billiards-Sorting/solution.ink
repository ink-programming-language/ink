// Translated from solution.cpp.

func REP(i: dynamic, b: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=b;i<n;i++)");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include<i");
}

var inf: dynamic = 100;

var N: dynamic = 5;

var dy: dynamic = [1, 1, -1, -1];

var dx: dynamic = [0, 1, 0, -1];

var wdy: dynamic = [-1, 1];

var edge: dynamic = cpp_array(N, N, 66605);

var wdcost: dynamic = cpp_array(66605, (N + 1));

class st
{
  var mat: dynamic = cpp_array(N, N);
  func st() -> dynamic
  {
      cpp_statement("rep(i,N)rep(j,N)mat[i][j] = 0; rep(i,N)");
      mat[i][i] = (N - i);
      mat[0][0] = (N - 1);
    }
  func operator_less(a: dynamic) -> dynamic
  {
      cpp_statement("rep(i,N)rep(j,N)");
      if ((mat[i][j] != a.mat[i][j]))
      {
        return (mat[i][j] < a.mat[i][j]);
      }
      return false;
    }
}

func getst(now: dynamic, M: dynamic) -> dynamic
{
  var index: dynamic = M.size();
  if ((M.count(now) == 0))
  {
    M[now] = index;
  }
  return M[now];
}

var NUM: dynamic = cpp_uninitialized();

func makeall() -> dynamic
{
  var M: dynamic = cpp_uninitialized();
  var Q: dynamic = cpp_uninitialized();
  var ini: dynamic = cpp_uninitialized();
  Q.push(ini);
  M.insert(make_pair(ini, 0));
  while ((!Q.empty()))
  {
    var now: dynamic = Q.front();
    Q.pop();
    var nownum: dynamic = getst(now, NUM);
    var tc: dynamic = M[now];
    wdcost[N][nownum] = tc;
  }
}

var row: dynamic = cpp_array(15);

var col: dynamic = cpp_array(15);

func precalcWD() -> dynamic
{
  var ori: dynamic = [1, 3, 6, 10, 15, 2, 5, 9, 14, 0, 4, 8, 13, 0, 0, 7, 12, 0, 0, 0, 11, 0, 0, 0, 0];
}

func getWD(n: dynamic, cpy: dynamic) -> dynamic
{
  if ((n != 5))
  {
    return make_pair(0, 0);
  }
  var r: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var inp: dynamic = [0];
  var matr: dynamic = [];
  var matc: dynamic = [];
  rep(i, N);
  rep(j, N).mat[i][j] = matr[i][j];
  rep(i, N);
  rep(j, N).mat[i][j] = matc[i][j];
  return make_pair(NUM[r], NUM[c]);
}

func initWD() -> dynamic
{
  rep(i, 66605);
  rep(j, N);
  rep(k, N)[i][j][k] = -1;
  makeall();
  precalcWD();
}

var mcost: dynamic = cpp_array(N, N, (N * N));

func precalc(n: dynamic) -> dynamic
{
  var pos: dynamic = cpp_array(n, n);
  var p: dynamic = 0;
  while (true)
  {
    var isupdate: dynamic = false;
    if ((!isupdate))
    {
      break;
    }
  }
}

var in_cpp: dynamic = cpp_array(N, N);

func geth(n: dynamic) -> dynamic
{
  cpp_statement("rep(i,n)rep(j,i+1)");
  mcost[0][i][j] = 0;
  var ret: dynamic = 0;
  return ret;
}

var ans: dynamic = cpp_uninitialized();

func solve(n: dynamic, cnt: dynamic, lim: dynamic, py: dynamic, px: dynamic, prev: dynamic, h: dynamic, wdr: dynamic, wdc: dynamic, y: dynamic, x: dynamic) -> dynamic
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
    var ney: dynamic = (y + dy[k]);
    var nex: dynamic = (x + dx[k]);
    if (((((ney == -1) || (nex == -1)) || (ney == n)) || (ney < nex)))
    {
      continue;
    }
    if ((in_cpp[ney][nex] == prev))
    {
      continue;
    }
    var nexth: dynamic = (h + ((mcost[(in_cpp[ney][nex] - 1)][y][x] - mcost[(in_cpp[ney][nex] - 1)][ney][nex])));
    var nextwdr: dynamic = wdr;
    var nextwdc: dynamic = wdc;
    if ((n == 5))
    {
      if (((k == 0) || (k == 2)))
      {
        assert((nex == x));
        var base: dynamic = x;
        nextwdr = edge[wdr][(ney - base)][row[in_cpp[ney][nex]]];
      } else
      {
        nextwdc = edge[wdc][nex][col[in_cpp[ney][nex]]];
      }
    }
    swap(in_cpp[y][x], in_cpp[ney][nex]);
    var ret: dynamic = cpp_uninitialized();
    ret = solve(n, (cnt + 1), lim, -1, -1, in_cpp[y][x], nexth, nextwdr, nextwdc, ney, nex);
    swap(in_cpp[y][x], in_cpp[ney][nex]);
    if (ret)
    {
      return true;
    }
  }
  return false;
}

func main() -> dynamic
{
  initWD();
  var n: dynamic = cpp_uninitialized();
  var tc: dynamic = 1;
  while (((cin >> n) && n))
  {
    ans = inf;
    precalc(n);
    var sy: dynamic = cpp_uninitialized();
    var sx: dynamic = cpp_uninitialized();
    rep(i, n);
    rep(j, (i + 1));
    if ((in_cpp[i][j] == 1))
    {
      sy = i;
      sx = j;
    }
    var mod: dynamic = mcost[0][sy][sx];
    var h: dynamic = geth(n);
    var beg: dynamic = 0;
    var wd: dynamic = getWD(n, in_cpp);
    var wdr: dynamic = wd.first;
    var wdc: dynamic = wd.second;
    if (((mod % 2) != 0))
    {
      beg += 1;
    }
    {
      var i: dynamic = beg;
      while (true)
      {
        ans = inf;
        var ret: dynamic = solve(n, 0, i, -1, -1, -1, h, wdr, wdc, sy, sx);
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

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        cpp_statement("rep(k,2)");
        {
          var ney: dynamic = (i + wdy[k]);
          if (((ney == -1) || (ney == N)))
          {
            continue;
          }
          if ((now.mat[ney][j] == 0))
          {
            continue;
          }
          var next: dynamic = now;
          next.mat[i][j] += 1;
          next.mat[ney][j] -= 1;
          var nextnum: dynamic = getst(next, NUM);
          edge[nownum][ney][j] = nextnum;
          if ((M.count(next) == 0))
          {
            M.insert(make_pair(next, (tc + 1)));
            Q.push(next);
          }
        }
      }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      if ((accumulate(now.mat[i], now.mat[(i + 1)], 0) == (N - i)))
      {
        continue;
      }
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      if (((ori[i][j] == 1) || (ori[i][j] == 0)))
      {
        continue;
      }
      row[ori[i][j]] = i;
      col[ori[i][j]] = j;
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
  }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      if ((cpy[i][j] == 0))
      {
        continue;
      }
      inp[cpp_update(p, "++")][j] = cpy[i][j];
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var p: dynamic = 0;
  }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      if (((inp[i][j] == 0) || (inp[i][j] == 1)))
      {
        continue;
      }
      matr[i][row[inp[i][j]]] += 1;
      matc[j][col[inp[i][j]]] += 1;
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
  }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
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

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
          cpp_statement("rep(j,i+1)");
          {
            cpp_statement("rep(k,4)");
            {
              var ney: dynamic = (i + dy[k]);
              var nex: dynamic = (j + dx[k]);
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

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      rep(jj, (ii + 1));
      {
        var now: dynamic = pos[ii][jj];
      }
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    cpp_statement("rep(j,i+1)");
    {
      ret += mcost[(in_cpp[i][j] - 1)][i][j];
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      cpp_statement("rep(j,i+1)");
      {
        read(in_cpp[i][j]);
      }
    }
