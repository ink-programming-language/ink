// Translated from solution.cpp.

var n: dynamic;

var a = cpp_array(1000005);

var add = cpp_array(1000005);

var k: dynamic;

var used = cpp_array(1000005);

func main()
{
  scanf("%lld%lld", (&n), (&k));
  var sum = ((n * ((n + 1))) / 2);
  if ((sum > k))
  {
    write(-1, "\n");
    return 0;
  }
  {
    var i = 1;
    while ((i <= n))
    {
      a[i] = i;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= (n / 2)))
    {
      var ch = ((n - (2 * i)) + 1);
      if (((sum + ch) <= k))
      {
        sum += ch;
        swap(a[i], a[((n - i) + 1)]);
      } else
      {
        swap(a[i], a[((k - sum) + i)]);
        sum = k;
        break;
      }
      i += 1;
    }
  }
  write(sum, "\n");
  {
    var i = 1;
    while ((i <= n))
    {
      write(i, cpp_char(" "));
      i += 1;
    }
  }
  write("\n");
  {
    var i = 1;
    while ((i <= n))
    {
      write(a[i], cpp_char(" "));
      i += 1;
    }
  }
  write("\n");
  return 0;
}
