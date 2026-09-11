// Translated from solution.cpp.

var exist: dynamic = cpp_array(200);

var a: dynamic = cpp_array(100005);

var b: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var tmp: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      tmp ^= a[i];
      var k: dynamic = 0;
      var t: dynamic = a[i];
      while (((t % 2) == 0))
      {
        t /= 2;
        k += 1;
      }
      exist[k] = true;
      i += 1;
    }
  }
  while ((tmp > 0))
  {
    b.push_back((tmp % 2));
    tmp /= 2;
  }
  var i: dynamic = (b.size() - 1);
  while ((i >= 0))
  {
    if (exist[i])
    {
      ans += 1;
    } else
    {
      write(-1, "\n");
      return 0;
    }
    while (((i >= 0) && (b[i] == 1)))
    {
      i -= 1;
    }
    if ((i < 0))
    {
      break;
    }
    if (exist[i])
    {
      ans += 1;
    } else
    {
      write(-1, "\n");
      return 0;
    }
    while (((i >= 0) && (b[i] == 0)))
    {
      i -= 1;
    }
    if ((i < 0))
    {
      break;
    }
  }
  write(ans, "\n");
}
