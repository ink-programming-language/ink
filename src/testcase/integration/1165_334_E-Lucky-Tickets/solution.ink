// Translated from solution.cpp.

var pre: dynamic = ["", "0", "00", "000"];

var T: dynamic = [1, 10, 100, 1000];

var k: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var v: dynamic = cpp_array(10000);

func ndgt(x: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < 3))
    {
      if ((x >= T[(3 - i)]))
      {
        return i;
      }
      i += 1;
    }
  }
  return 3;
}

func print(x: dynamic, s: dynamic) -> dynamic
{
  var y: dynamic =  ((k < s)) ? (s - k) : (k - s);
  if (((y < 10000) && (!v[y])))
  {
    write(pre[ndgt(y)], y, pre[ndgt(x)], x, "\n");
    v[y] = true;
    return true;
  }
  return false;
}

func calc(t: dynamic, x: dynamic) -> dynamic
{
  var p: dynamic = 1;
  var b: dynamic = 0;
  var a: dynamic = 0;
  var op: dynamic = 0;
  var z: dynamic = x;
  {
    var i: dynamic = 0;
    while ((i < 4))
    {
      var op: dynamic = (t % 4);
      t /= 4;
      b += (((z % 10)) * p);
      if ((op == 3))
      {
        p *= 10;
      } else
      {
        var __cpp_switch_1: dynamic = op;
        if (__cpp_switch_1 == 0)
        {
          a = (a + b);
          break;
        }
        else if (__cpp_switch_1 == 1)
        {
          a = (a - b);
          break;
        }
        else if (__cpp_switch_1 == 2)
        {
          a = (a * b);
          break;
        }
        op = op;
        b = 0;
        p = 1;
      }
      z /= 10;
      i += 1;
    }
  }
  return a;
}

var rep: dynamic = cpp_array(64);

func main1() -> dynamic
{
  var cpp_1: dynamic = cpp_uninitialized();
  var cpp_2: dynamic = cpp_uninitialized();
  while (((cin >> cpp_1) >> cpp_2))
  {
    write(calc(cpp_1, cpp_2), "\n");
  }
  return 0;
}

func main() -> dynamic
{
  while (((cin >> k) >> m))
  {
    {
      var x: dynamic = 0;
      while (((x <= 9999) && m))
      {
        memset(v, false, cpp_sizeof((v)));
        {
          var cb: dynamic = 0;
          while ((cb < 64))
          {
            var s: dynamic = calc(cb, x);
            rep[cb] = s;
            cb += 1;
          }
        }
        sort(rep, (rep + 64));
        if (print(x, rep[0]))
        {
          m -= 1;
        }
        {
          var i: dynamic = 1;
          while (((i < 64) && m))
          {
            if ((rep[i] > rep[(i - 1)]))
            {
              if (print(x, rep[i]))
              {
                m -= 1;
              }
            }
            i += 1;
          }
        }
        x += 1;
      }
    }
  }
  return 0;
}

func main2() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  var cnt: dynamic = 0;
  while (getline(cin, s))
  {
    cnt += 1;
    if ((s.size() != 8))
    {
      write(s, "\n");
    }
  }
  write("cnt=", cnt, "\n");
  return 0;
}
