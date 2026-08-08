// Translated from solution.cpp.

func REP(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a;i<(int)b;i++)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <");
}

var EPS = cpp_expression("#inclu");

var q = -999999999;

func gauss_jordan(A: dynamic, b: dynamic)
{
  var n = A.size();
  var B = cpp_construct(n, vec((n + 1)));
  rep(i, n);
  rep(j, n)[i][j] = A[i][j];
  rep(i, n)[i][n] = b[i];
  rep(i, n)[i] = B[i][n];
  return x;
}

var N: dynamic;

var M: dynamic;

var in_cpp: dynamic;

var qpos: dynamic;

var fail = cpp_expression("#include <iostream> #include <");

func main()
{
  var first = 0;
  while ((((cin >> N) >> M) && N))
  {
    if (first)
    {
      write("\n");
    }
    first = 1;
    var remain = 0;
    in_cpp.clear();
    in_cpp.resize((N + 1));
    rep(i, (N + 1))[i].resize((M + 1));
    qpos.clear();
    rep(i, (N + 1));
    rep(j, (M + 1));
    {
      var s: dynamic;
      read(s);
      if ((s == "?"))
      {
        in_cpp[i][j] = q;
        qpos.emplace_back(i, j);
      } else
      {
        in_cpp[i][j] = stoi(s);
      }
    }
    rep(cpp_name, ((((N + 1)) * ((M + 1))) + 10));
    {
      {
        rep(i, (N + 1));
        {
          var xcnt = 0;
          var sum = 0;
          var lastq = -1;
          if ((xcnt == 0))
          {
            if ((in_cpp[i][M] == q))
            {
              in_cpp[i][M] = sum;
            } else if ((in_cpp[i][M] != sum))
            {
              fail;
            }
          } else if ((xcnt == 1))
          {
            if ((in_cpp[i][M] == q))
            {
            } else
            {
              in_cpp[i][lastq] = (in_cpp[i][M] - sum);
            }
          } else
          {
          }
        }
      }
      {
        rep(j, (M + 1));
        {
          var xcnt = 0;
          var sum = 0;
          var lastq = -1;
          if ((xcnt == 0))
          {
            if ((in_cpp[N][j] == q))
            {
              in_cpp[N][j] = sum;
            } else if ((in_cpp[N][j] != sum))
            {
              fail;
            }
          } else if ((xcnt == 1))
          {
            if ((in_cpp[N][j] == q))
            {
            } else
            {
              in_cpp[lastq][j] = (in_cpp[N][j] - sum);
            }
          } else
          {
          }
        }
      }
    }
    rep(i, (N + 1));
    rep(j, (M + 1));
    {
      remain |= (in_cpp[i][j] == q);
    }
    if (remain)
    {
      fail;
    }
    for (var e in qpos)
    {
      write(in_cpp[e.first][e.second], "\n");
    }
  }
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic, argument_2: dynamic)
{
      if ((abs(B[j][i]) > abs(B[pivot][i])))
      {
        pivot = j;
      }
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      if ((i != j))
      {
        REP(k, (i + 1), (n + 1))[j][k] -= (B[j][i] * B[i][k]);
      }
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var pivot = i;
    swap(B[i], B[pivot]);
    if ((abs(B[i][i]) < EPS))
    {
      return vec();
    }
    REP(j, (i + 1), (n + 1))[i][j] /= B[i][i];
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
            if ((in_cpp[i][j] == q))
            {
              xcnt += 1;
              lastq = j;
            } else
            {
              sum += in_cpp[i][j];
            }
          }

func rep(argument_0: dynamic, argument_1: dynamic)
{
            if ((in_cpp[i][j] == q))
            {
              xcnt += 1;
              lastq = i;
            } else
            {
              sum += in_cpp[i][j];
            }
          }
