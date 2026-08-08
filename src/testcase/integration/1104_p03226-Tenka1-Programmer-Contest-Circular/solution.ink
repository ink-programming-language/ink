// Translated from solution.cpp.

var dibs = cpp_expression("#includ");

var OVER9000 = cpp_expression("#include <");

func ALL_THE(CAKE: dynamic, LIE: dynamic)
{
  cpp_macro("for(auto LIE =CAKE.begin(); LIE != CAKE.end(); LIE++)");
}

var tisic = cpp_expression("#i");

var soclose = cpp_expression("#inc");

var chocolate = cpp_expression("#in");

var patkan = cpp_expression("#");

var ff = cpp_expression("#incl");

var ss = cpp_expression("#inclu");

func abs(x: dynamic)
{
  return cpp_expression("#include <bits/stdc+");
}

var uint_cpp = dynamic;

var dbl = dynamic;

var pi = cpp_expression("#include <bits/stdc++.");

var lld = cpp_expression("#inc");

func get_min(RMQ: dynamic, l: dynamic, r: dynamic)
{
  var ret = OVER9000;
  {
    var i = 0;
    while ((i < 19))
    {
      if ((((((r - l)) >> i)) & 1))
      {
        ret = min(ret, RMQ[i][l]);
        l += (1 << i);
        if ((l == r))
        {
          break;
        }
      }
      i += 1;
    }
  }
  return ret;
}

func get_max(RMQ: dynamic, l: dynamic, r: dynamic)
{
  var ret = -1;
  {
    var i = 0;
    while ((i < 19))
    {
      if ((((((r - l)) >> i)) & 1))
      {
        ret = max(ret, RMQ[i][l]);
        l += (1 << i);
        if ((l == r))
        {
          break;
        }
      }
      i += 1;
    }
  }
  return ret;
}

func main()
{
  cin.sync_with_stdio(0);
  cin.tie(0);
  write(fixed, setprecision(10));
  var N: dynamic;
  read(N);
  var cnt = cpp_construct(N, 0);
  {
    var i = 0;
    while ((i < N))
    {
      read(A[i]);
      cnt[cpp_update(A[i], "--")] += 1;
      i += 1;
    }
  }
  var K = cnt[0];
  if ((K == 0))
  {
    write("0\n");
    return 0;
  }
  if ((cnt[0] == N))
  {
    var ans = 1;
    var mod = 998244353;
    {
      var i = 0;
      while ((i < N))
      {
        ans = ((ans * ((i + 1))) % mod);
        i += 1;
      }
    }
    write(ans, "\n");
    return 0;
  }
  {
    var i = 0;
    while ((i < N))
    {
      occ[A[i]].push_back(i);
      i += 1;
    }
  }
  first_idx[0] = occ[0][0];
  while ((A[((((first_idx[0] + N) - 1)) % N)] == 0))
  {
    first_idx[0] = ((((first_idx[0] + N) - 1)) % N);
  }
  {
    var i = 1;
    while ((i < N))
    {
      if (cnt[i])
      {
        if ((cnt[i] == 1))
        {
          first_idx[i] = occ[i][0];
          i += 1;
          continue;
        }
        var x = 0;
        {
          var j = 0;
          while ((j < cpp_cast(occ[i].size())))
          {
            var d = (occ[i][(((j + 1)) % occ[i].size())] - occ[i][j]);
            while ((d < 0))
            {
              d += N;
            }
            while ((d >= N))
            {
              d -= N;
            }
            if ((d >= (N - (N / 2))))
            {
              first_idx[i] = occ[i][(((j + 1)) % occ[i].size())];
              x += 1;
            }
            j += 1;
          }
        }
        if ((x != 1))
        {
          write("0\n");
          return 0;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      if (cnt[i])
      {
        {
          var j = 0;
          while ((j < cpp_cast(occ[i].size())))
          {
            if ((occ[i][j] == first_idx[i]))
            {
              var occ_nw: dynamic;
              {
                var k = 0;
                while ((k < cpp_cast(occ[i].size())))
                {
                  occ_nw.push_back(occ[i][(((j + k)) % occ[i].size())]);
                  k += 1;
                }
              }
              occ[i] = occ_nw;
              break;
            }
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < K))
    {
      if ((occ[0][i] != (((first_idx[0] + i)) % N)))
      {
        write("0\n");
        return 0;
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      if (cnt[i])
      {
        var x = ((occ[i].back() - K) + 1);
        while ((x < 0))
        {
          x += N;
        }
        x %= N;
        R[i] = [x, occ[i][0]];
        var d = (occ[i].back() - occ[i][0]);
        while ((d < 0))
        {
          d += N;
        }
        d %= N;
        if ((d >= K))
        {
          write("0\n");
          return 0;
        }
      }
      i += 1;
    }
  }
  var RMQ_max = cpp_construct(19, vector(N));
  RMQ_max[0] = A;
  {
    var i = 1;
    while ((i < 19))
    {
      {
        var j = 0;
        while ((j < N))
        {
          RMQ_max[i][j] = max(RMQ_max[(i - 1)][j], RMQ_max[(i - 1)][min((N - 1), (j + ((1 << ((i - 1))))))]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      minval[i] = get_max(RMQ_max, i, min(N, (i + K)));
      if (((i + K) > N))
      {
        minval[i] = max(minval[i], get_max(RMQ_max, 0, ((i + K) - N)));
      }
      i += 1;
    }
  }
  var RMQ_min = cpp_construct(19, vector(N));
  RMQ_min[0] = minval;
  {
    var i = 1;
    while ((i < 19))
    {
      {
        var j = 0;
        while ((j < N))
        {
          RMQ_min[i][j] = min(RMQ_min[(i - 1)][j], RMQ_min[(i - 1)][min((N - 1), (j + ((1 << ((i - 1))))))]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      if (cnt[i])
      {
        var r = if (((R[i].ss >= R[i].ff))) R[i].ss else ((N - 1));
        {
          var j = 18;
          while ((j >= 0))
          {
            while (((R[i].ff + ((1 << j))) <= r))
            {
              if ((RMQ_min[j][R[i].ff] <= i))
              {
                break;
              }
              R[i].ff += (1 << j);
            }
            j -= 1;
          }
        }
        {
          var j = 0;
          while ((j < 3))
          {
            if ((minval[R[i].ff] <= i))
            {
              break;
            }
            if ((R[i].ff == R[i].ss))
            {
              write("0\n");
              return 0;
            }
            R[i].ff = (((R[i].ff + 1)) % N);
            j += 1;
          }
        }
        r = if (((R[i].ss >= R[i].ff))) R[i].ss else ((N - 1));
        {
          var j = 18;
          while ((j >= 0))
          {
            while (((R[i].ff + ((1 << j))) <= r))
            {
              if ((RMQ_min[j][R[i].ff] <= i))
              {
                break;
              }
              R[i].ff += (1 << j);
            }
            j -= 1;
          }
        }
        var l = if (((R[i].ff <= R[i].ss))) R[i].ff else 0;
        {
          var j = 18;
          while ((j >= 0))
          {
            while (((R[i].ss - ((1 << j))) >= l))
            {
              if ((RMQ_min[j][((R[i].ss - ((1 << j))) + 1)] <= i))
              {
                break;
              }
              R[i].ss -= (1 << j);
            }
            j -= 1;
          }
        }
        {
          var j = 0;
          while ((j < 3))
          {
            if ((minval[R[i].ss] <= i))
            {
              break;
            }
            if ((R[i].ff == R[i].ss))
            {
              write("0\n");
              return 0;
            }
            R[i].ss = ((((R[i].ss + N) - 1)) % N);
            j += 1;
          }
        }
        l = if (((R[i].ff <= R[i].ss))) R[i].ff else 0;
        {
          var j = 18;
          while ((j >= 0))
          {
            while (((R[i].ss - ((1 << j))) >= l))
            {
              if ((RMQ_min[j][((R[i].ss - ((1 << j))) + 1)] <= i))
              {
                break;
              }
              R[i].ss -= (1 << j);
            }
            j -= 1;
          }
        }
      }
      i += 1;
    }
  }
  var cnt_pos0 = cpp_construct(N, 0);
  {
    var i = 0;
    while ((i < N))
    {
      cnt_pos0[minval[i]] += 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < N))
    {
      cnt_pos0[i] += cnt_pos0[(i - 1)];
      i += 1;
    }
  }
  var ans = 1;
  var mod = 998244353;
  {
    var i = 0;
    while ((i < N))
    {
      if ((cnt[i] == 0))
      {
        if ((cnt_pos0[i] <= i))
        {
          write("0\n");
          return 0;
        }
        ans = ((ans * ((cnt_pos0[i] - i))) % mod);
        i += 1;
        continue;
      }
      var d = (R[i].ss - R[i].ff);
      while ((d < 0))
      {
        d += N;
      }
      d %= N;
      ans = ((ans * ((d + 1))) % mod);
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
