// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var sum: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

var num: dynamic = cpp_array(60010);

func main() -> dynamic
{
  read(n);
  var k: dynamic = ((((1 * n) * ((n + 1))) / 2));
  if ((k & 1))
  {
    write(1, "\n");
    sum = ((k / 2) + 1);
  } else
  {
    write(0, "\n");
    sum = (k / 2);
  }
  {
    var i: dynamic = n;
    while ((i >= 1))
    {
      if (((sum - i) > 0))
      {
        sum -= i;
        num[cpp_update(cnt, "++")] = i;
      } else if ((sum == i))
      {
        sum -= i;
        num[cpp_update(cnt, "++")] = i;
        break;
      }
      i -= 1;
    }
  }
  write(cnt, cpp_char(" "));
  {
    var i: dynamic = 1;
    while ((i <= cnt))
    {
      write(num[i], cpp_char(" "));
      i += 1;
    }
  }
  write("\n");
  return 0;
}
