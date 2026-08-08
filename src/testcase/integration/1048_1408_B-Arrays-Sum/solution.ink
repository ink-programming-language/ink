// Translated from solution.cpp.

var a = cpp_array(105);

var mark = cpp_array(105);

func main()
{
  var n: dynamic;
  var k: dynamic;
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n, k);
    var cnt = 0;
    {
      var i = 0;
      while ((i < n))
      {
        read(a[i]);
        if ((!mark[a[i]]))
        {
          cnt += 1;
          mark[a[i]] = 1;
        }
        i += 1;
      }
    }
    var ans = 0;
    if ((cnt <= k))
    {
      write(1, "\n");
    } else if ((k == 1))
    {
      if ((cnt == 1))
      {
        write(1, "\n");
      } else
      {
        write(-1, "\n");
      }
    } else
    {
      cnt -= k;
      if ((cnt == 0))
      {
        write(1, "\n");
      } else if ((cnt <= ((k - 1))))
      {
        write(2, "\n");
      } else if ((cnt % ((k - 1))))
      {
        write(((cnt / ((k - 1))) + 2), "\n");
      } else
      {
        write(((cnt / ((k - 1))) + 1), "\n");
      }
    }
    memset(mark, 0, cpp_sizeof((mark)));
  }
  return 0;
}
