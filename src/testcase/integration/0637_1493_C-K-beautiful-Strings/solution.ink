// Translated from solution.cpp.

var MAX: dynamic = (1e5 + 10);

var need: dynamic = cpp_array(27);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

func check(idx: dynamic) -> dynamic
{
  memset(need, 0, cpp_sizeof((need)));
  {
    var i: dynamic = 0;
    while ((i < idx))
    {
      need[(s[i] - cpp_char("a"))] = ((((need[(s[i] - cpp_char("a"))] - 1) + k)) % k);
      i += 1;
    }
  }
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < 26))
    {
      sum += need[i];
      i += 1;
    }
  }
  if ((idx == n))
  {
    return ((sum == 0));
  }
  {
    var c: dynamic = (s[idx] + 1);
    while ((c <= cpp_char("z")))
    {
      sum -= need[(c - cpp_char("a"))];
      need[(c - cpp_char("a"))] = ((((need[(c - cpp_char("a"))] - 1) + k)) % k);
      sum += need[(c - cpp_char("a"))];
      var x: dynamic = (((n - idx) - 1) - sum);
      if (((x >= 0) && ((x % k) == 0)))
      {
        return true;
      }
      sum -= need[(c - cpp_char("a"))];
      need[(c - cpp_char("a"))] = (((need[(c - cpp_char("a"))] + 1)) % k);
      sum += need[(c - cpp_char("a"))];
      c += 1;
    }
  }
  return false;
}

func build(idx: dynamic) -> dynamic
{
  memset(need, 0, cpp_sizeof((need)));
  {
    var i: dynamic = 0;
    while ((i < idx))
    {
      need[(s[i] - cpp_char("a"))] = ((((need[(s[i] - cpp_char("a"))] - 1) + k)) % k);
      i += 1;
    }
  }
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < 26))
    {
      sum += need[i];
      i += 1;
    }
  }
  if ((idx == n))
  {
    write(s, "\n");
    return;
  }
  {
    var c: dynamic = (s[idx] + 1);
    while ((c <= cpp_char("z")))
    {
      sum -= need[(c - cpp_char("a"))];
      need[(c - cpp_char("a"))] = ((((need[(c - cpp_char("a"))] - 1) + k)) % k);
      sum += need[(c - cpp_char("a"))];
      var x: dynamic = (((n - idx) - 1) - sum);
      if (((x >= 0) && ((x % k) == 0)))
      {
        {
          var i: dynamic = 0;
          while ((i < idx))
          {
            write(s[i]);
            i += 1;
          }
        }
        write(c);
        {
          var i: dynamic = 0;
          while ((i < x))
          {
            write(cpp_char("a"));
            i += 1;
          }
        }
        {
          var i: dynamic = 0;
          while ((i < 26))
          {
            {
              var j: dynamic = 0;
              while ((j < need[i]))
              {
                write(cpp_cast(((cpp_char("a") + i))));
                j += 1;
              }
            }
            i += 1;
          }
        }
        write("\n");
        return;
      }
      sum -= need[(c - cpp_char("a"))];
      need[(c - cpp_char("a"))] = (((need[(c - cpp_char("a"))] + 1)) % k);
      sum += need[(c - cpp_char("a"))];
      c += 1;
    }
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n, k);
    read(s);
    if (((n % k) != 0))
    {
      write(-1, "\n");
      continue;
    }
    if (check(n))
    {
      write(s, "\n");
      continue;
    }
    memset(need, 0, cpp_sizeof((need)));
    var sum: dynamic = 0;
    var idx: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        {
          var c: dynamic = (s[i] + 1);
          while ((c <= cpp_char("z")))
          {
            sum -= need[(c - cpp_char("a"))];
            need[(c - cpp_char("a"))] = ((((need[(c - cpp_char("a"))] - 1) + k)) % k);
            sum += need[(c - cpp_char("a"))];
            var x: dynamic = (((n - i) - 1) - sum);
            if (((x >= 0) && ((x % k) == 0)))
            {
              idx = i;
            }
            sum -= need[(c - cpp_char("a"))];
            need[(c - cpp_char("a"))] = (((need[(c - cpp_char("a"))] + 1)) % k);
            sum += need[(c - cpp_char("a"))];
            c += 1;
          }
        }
        sum -= need[(s[i] - cpp_char("a"))];
        need[(s[i] - cpp_char("a"))] = ((((need[(s[i] - cpp_char("a"))] - 1) + k)) % k);
        sum += need[(s[i] - cpp_char("a"))];
        i += 1;
      }
    }
    build(idx);
  }
  return 0;
}
