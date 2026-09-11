// Translated from solution.cpp.

var N: dynamic = (2e6 + 5);

var MOD: dynamic = (1e9 + 7);

var INF: dynamic = 9e18;

var LLL: dynamic = (1 << 15);

var buffer: dynamic = cpp_array((LLL + 5));

var s: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

func get_char() -> dynamic
{
  if ((s == t))
  {
    t = ((cpp_assign(s, "=", buffer)) + fread(buffer, 1, LLL, stdin));
    if ((s == t))
    {
      return EOF;
    }
  }
  return (*cpp_update(s, "++"));
}

func get_int() -> dynamic
{
  var c: dynamic = cpp_uninitialized();
  var flg: dynamic = 1;
  var ret: dynamic = 0;
  while (cpp_comma(cpp_assign(c, "=", get_char()), ((c < cpp_char("0")) || (c > cpp_char("9")))))
  {
    if ((c == cpp_char("-")))
    {
      flg = -1;
    }
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    ret = ((((ret << 1)) + ((ret << 3))) + ((c - cpp_char("0"))));
    c = get_char();
  }
  return (ret * flg);
}

func get_LL() -> dynamic
{
  var c: dynamic = cpp_uninitialized();
  var flg: dynamic = 1;
  var ret: dynamic = 0;
  while (cpp_comma(cpp_assign(c, "=", get_char()), ((c < cpp_char("0")) || (c > cpp_char("9")))))
  {
    if ((c == cpp_char("-")))
    {
      flg = -1;
    }
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    ret = ((((ret << 1)) + ((ret << 3))) + ((c - cpp_char("0"))));
    c = get_char();
  }
  return (ret * flg);
}

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
}

var p: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var now: dynamic = cpp_uninitialized();

func check(mid: dynamic) -> dynamic
{
  var l: dynamic = [0, (-INF), (-INF), (-INF), (-INF)];
  var r: dynamic = [0, INF, INF, INF, INF];
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      l[1] = max(l[1], (((p[i].x + p[i].y) + p[i].z) - mid));
      r[1] = min(r[1], (((p[i].x + p[i].y) + p[i].z) + mid));
      l[2] = max(l[2], (((p[i].x + p[i].y) - p[i].z) - mid));
      r[2] = min(r[2], (((p[i].x + p[i].y) - p[i].z) + mid));
      l[3] = max(l[3], (((p[i].x - p[i].y) + p[i].z) - mid));
      r[3] = min(r[3], (((p[i].x - p[i].y) + p[i].z) + mid));
      l[4] = max(l[4], ((((-p[i].x) + p[i].y) + p[i].z) - mid));
      r[4] = min(r[4], ((((-p[i].x) + p[i].y) + p[i].z) + mid));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 2))
    {
      var ll: dynamic = cpp_array(5);
      var rr: dynamic = cpp_array(5);
      var suc: dynamic = 1;
      {
        var i: dynamic = 0;
        while ((i < 5))
        {
          ll[i] = l[i];
          rr[i] = r[i];
          i += 1;
        }
      }
      {
        var j: dynamic = 1;
        while ((j <= 4))
        {
          if ((((ll[j] & 1)) == ((i ^ 1))))
          {
            ll[j] += 1;
          }
          if ((((rr[j] & 1)) == ((i ^ 1))))
          {
            rr[j] -= 1;
          }
          if ((ll[j] > rr[j]))
          {
            suc = 0;
          }
          j += 1;
        }
      }
      if ((((!suc) || (((rr[2] + rr[3]) + rr[4]) < ll[1])) || (((ll[2] + ll[3]) + ll[4]) > rr[1])))
      {
        i += 1;
        continue;
      }
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      var c: dynamic = cpp_uninitialized();
      var inc: dynamic = cpp_uninitialized();
      a = ll[2];
      b = ll[3];
      c = ll[4];
      inc = max(0, (((ll[1] - a) - b) - c));
      var t: dynamic = cpp_uninitialized();
      t = min((rr[2] - ll[2]), inc);
      a += t;
      inc -= t;
      t = min((rr[3] - ll[3]), inc);
      b += t;
      inc -= t;
      t = min((rr[4] - ll[4]), inc);
      c += t;
      inc -= t;
      if (inc)
      {
        i += 1;
        continue;
      }
      var ax: dynamic = cpp_uninitialized();
      var ay: dynamic = cpp_uninitialized();
      var az: dynamic = cpp_uninitialized();
      ax = ((a + b) >> 1);
      ay = ((a + c) >> 1);
      az = ((b + c) >> 1);
      now = [ax, ay, az];
      return 1;
      i += 1;
    }
  }
  return 0;
}

func Solve(cas: dynamic = 0) -> dynamic
{
  n = get_int();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      p[i].x = get_LL();
      p[i].y = get_LL();
      p[i].z = get_LL();
      i += 1;
    }
  }
  var l: dynamic = 0;
  var r: dynamic = 3e18;
  var mid: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
  while ((l <= r))
  {
    mid = (((l + r)) >> 1);
    if (check(mid))
    {
      r = (mid - 1);
      ans = now;
    } else
    {
      l = (mid + 1);
    }
  }
  printf("%lld %lld %lld\n", ans.x, ans.y, ans.z);
  return 0;
}

func Pre() -> dynamic
{
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  Pre();
  var cas: dynamic = cpp_uninitialized();
  cas = get_int();
  {
    var i: dynamic = 1;
    while ((i <= cas))
    {
      Solve(i);
      i += 1;
    }
  }
  return 0;
}
