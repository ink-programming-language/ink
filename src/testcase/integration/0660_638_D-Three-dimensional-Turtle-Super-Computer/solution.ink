// Translated from solution.cpp.

var INF: dynamic = 1e18;

var INFi: dynamic = (1e9 * 2);

var maxN: dynamic = 100;

var P: dynamic = 998244353;

var md: dynamic = (1e9 + 7);

func getTime() -> dynamic
{
  return (clock() / cpp_cast(CLOCKS_PER_SEC));
}

func __cpp_top_level_1() -> dynamic
{
}

var state: dynamic = cpp_array(maxN, maxN, maxN);

var di: dynamic = [0, 0, 1];

var dj: dynamic = [0, 1, 0];

var de: dynamic = [1, 0, 0];

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

func check(i: dynamic, j: dynamic, e: dynamic) -> dynamic
{
  return (!(cpp_assign((((i < 0) || (i >= n)) || j), "=", ((m || (e < 0)) || (e >= k)))));
}

func check_path(i: dynamic, j: dynamic, e: dynamic, fi: dynamic, fj: dynamic, fe: dynamic) -> dynamic
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
    var t: dynamic = 0;
    while (((t) < 3))
    {
      var new_i: dynamic = (i + di[t]);
      var new_j: dynamic = (j + dj[t]);
      var new_e: dynamic = (e + de[t]);
      if (check_path(new_i, new_j, new_e, fi, fj, fe))
      {
        return true;
      }
      (t) += 1;
    }
  }
  return false;
}

var ans: dynamic = 0;

func checked(i: dynamic, j: dynamic, e: dynamic) -> dynamic
{
  if ((state[i][j][e] == 0))
  {
    return 0;
  }
  {
    var t1: dynamic = 0;
    while (((t1) < 3))
    {
      {
        var t2: dynamic = 0;
        while (((t2) < 3))
        {
          var i_start: dynamic = (i - di[t1]);
          var j_start: dynamic = (j - dj[t1]);
          var e_start: dynamic = (e - de[t1]);
          var i_finish: dynamic = (i + di[t2]);
          var j_finish: dynamic = (j + dj[t2]);
          var e_finish: dynamic = (e + de[t2]);
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

func solve() -> dynamic
{
  read(n, m, k);
  {
    var i: dynamic = 0;
    while (((i) < n))
    {
      {
        var j: dynamic = 0;
        while (((j) < m))
        {
          {
            var e: dynamic = 0;
            while (((e) < k))
            {
              var a: dynamic = cpp_uninitialized();
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
    var i: dynamic = 0;
    while (((i) < n))
    {
      {
        var j: dynamic = 0;
        while (((j) < m))
        {
          {
            var e: dynamic = 0;
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

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var tests: dynamic = 1;
  {
    var cpp_name: dynamic = 0;
    while (((cpp_name) < tests))
    {
      solve();
      (cpp_name) += 1;
    }
  }
  return 0;
}
