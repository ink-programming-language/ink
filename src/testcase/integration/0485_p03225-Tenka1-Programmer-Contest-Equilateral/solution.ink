// Translated from solution.cpp.

var MAXN = 305;

var N: dynamic;

var M: dynamic;

var ps = cpp_array((3 * MAXN), (3 * MAXN));

var ps2 = cpp_array((3 * MAXN), (3 * MAXN));

var arr = cpp_array(MAXN, MAXN);

var utama = cpp_array((2 * MAXN));

var lain = cpp_array((2 * MAXN));

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(N, M);
  {
    var i = 0;
    while ((i < N))
    {
      {
        var j = 0;
        while ((j < M))
        {
          read(arr[i][j]);
          if ((arr[i][j] == cpp_char("#")))
          {
            utama[(i + j)].push_back([i, j]);
            lain[((i - j) + MAXN)].push_back([i, j]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = (-N);
    while ((i < (2 * N)))
    {
      {
        var j = (-M);
        while ((j < (2 * M)))
        {
          ps[(i + MAXN)][(j + MAXN)] = ps[((i - 1) + MAXN)][((j + 1) + MAXN)];
          ps2[(i + MAXN)][(j + MAXN)] = ps2[((i - 1) + MAXN)][((j - 1) + MAXN)];
          if ((((((0 <= i) && (i < N)) && (0 <= j)) && (j < M)) && (arr[i][j] == cpp_char("#"))))
          {
            ps[(i + MAXN)][(j + MAXN)] += 1;
            ps2[(i + MAXN)][(j + MAXN)] += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var miring = 0;
    while ((miring < (N + M)))
    {
      {
        var i = 0;
        while ((i < cpp_cast(utama[miring].size())))
        {
          {
            var j = (i + 1);
            while ((j < cpp_cast(utama[miring].size())))
            {
              var p = utama[miring][i];
              var q = utama[miring][j];
              var x = (((p.second - q.second) + q.first) - p.first);
              assert((x > 0));
              ans += (ps[(p.first + MAXN)][(((miring - x) - p.first) + MAXN)] - ps[(((miring - x) - ((q.second + 1))) + MAXN)][((q.second + 1) + MAXN)]);
              ans += (ps[(((miring + x) - p.second) + MAXN)][(p.second + MAXN)] - ps[((q.first - 1) + MAXN)][(((miring + x) - ((q.first - 1))) + MAXN)]);
              j += 1;
            }
          }
          i += 1;
        }
      }
      miring += 1;
    }
  }
  {
    var miring = (-M);
    while ((miring < N))
    {
      {
        var i = 0;
        while ((i < cpp_cast(lain[(miring + MAXN)].size())))
        {
          {
            var j = (i + 1);
            while ((j < cpp_cast(lain[(miring + MAXN)].size())))
            {
              var p = lain[(miring + MAXN)][i];
              var q = lain[(miring + MAXN)][j];
              var x = (((q.first - p.first) + q.second) - p.second);
              assert((x > 0));
              ans += (ps2[((((miring + x) + p.second) - 1) + MAXN)][((p.second - 1) + MAXN)] - ps2[(q.first + MAXN)][((((-miring) - x) + q.first) + MAXN)]);
              ans += (ps2[((p.first - 1) + MAXN)][((((-miring) + x) + ((p.first - 1))) + MAXN)] - ps2[(((miring - x) + q.second) + MAXN)][(q.second + MAXN)]);
              j += 1;
            }
          }
          i += 1;
        }
      }
      miring += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
