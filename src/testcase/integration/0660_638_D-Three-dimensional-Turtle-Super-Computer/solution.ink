// Translated from solution.cpp.

var INF = 1e18;

var INFi = (1e9 * 2);

var maxN = 100;

var P = 998244353;

var md = (1e9 + 7);

func getTime()
{
  return (clock() / cpp_cast(CLOCKS_PER_SEC));
}

func __cpp_top_level_1()
{
}

var state = cpp_array(maxN, maxN, maxN);

var di = [0, 0, 1];

var dj = [0, 1, 0];

var de = [1, 0, 0];

var n: dynamic;

var m: dynamic;

var k: dynamic;

func check(i: dynamic, j: dynamic, e: dynamic)
{
  return (!(cpp_assign((((i < 0) || (i >= n)) || j), "=", ((m || (e < 0)) || (e >= k)))));
}

func check_path(i: dynamic, j: dynamic, e: dynamic, fi: dynamic, fj: dynamic, fe: dynamic)
{
  if (((!check(i, j, e)) || (!check(fi, fj, fe))))
  {
    return false;
  }
  if ((((i > fi) || (j > fj)) || (e > fe)))
  {
    return false;
  }
  if ((state[i][j][e] == 0))
  {
    return false;
  }
  if ((((i == fi) && (j == fj)) && (e == fe)))
  {
    return true;
  }
  {
    var t = 0;
    while (((t) < 3))
    {
      var new_i = (i + di[t]);
      var new_j = (j + dj[t]);
      var new_e = (e + de[t]);
      if (check_path(new_i, new_j, new_e, fi, fj, fe))
      {
        return true;
      }
      (t) += 1;
    }
  }
  return false;
}

var ans = 0;

func checked(i: dynamic, j: dynamic, e: dynamic)
{
  if ((state[i][j][e] == 0))
  {
    return 0;
  }
  {
    var t1 = 0;
    while (((t1) < 3))
    {
      {
        var t2 = 0;
        while (((t2) < 3))
        {
          var i_start = (i - di[t1]);
          var j_start = (j - dj[t1]);
          var e_start = (e - de[t1]);
          var i_finish = (i + di[t2]);
          var j_finish = (j + dj[t2]);
          var e_finish = (e + de[t2]);
          if (check_path(i_start, j_start, e_start, i_finish, j_finish, e_finish))
          {
            state[i][j][e] = 0;
            if ((!check_path(i_start, j_start, e_start, i_finish, j_finish, e_finish)))
            {
              state[i][j][e] = 1;
              return 1;
            }
            state[i][j][e] = 1;
          }
          (t2) += 1;
        }
      }
      (t1) += 1;
    }
  }
  return 0;
}

func solve()
{
  read(n, m, k);
  {
    var i = 0;
    while (((i) < n))
    {
      {
        var j = 0;
        while (((j) < m))
        {
          {
            var e = 0;
            while (((e) < k))
            {
              var a: dynamic;
              read(a);
              state[i][j][e] = (a - cpp_char("0"));
              (e) += 1;
            }
          }
          (j) += 1;
        }
      }
      (i) += 1;
    }
  }
  {
    var i = 0;
    while (((i) < n))
    {
      {
        var j = 0;
        while (((j) < m))
        {
          {
            var e = 0;
            while (((e) < k))
            {
              ans += checked(i, j, e);
              (e) += 1;
            }
          }
          (j) += 1;
        }
      }
      (i) += 1;
    }
  }
  write(ans);
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var tests = 1;
  {
    var cpp_name = 0;
    while (((cpp_name) < tests))
    {
      solve();
      (cpp_name) += 1;
    }
  }
  return 0;
}
