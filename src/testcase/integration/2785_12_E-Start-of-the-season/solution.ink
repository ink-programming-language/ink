// Translated from solution.cpp.

var lastrow = cpp_array(1002);

func main()
{
  var n: dynamic;
  var r: dynamic;
  var c: dynamic;
  var p: dynamic;
  var remember: dynamic;
  read(n);
  lastrow[0] = (n - 1);
  lastrow[(n - 1)] = 0;
  {
    r = 0;
    while ((r < (n - 1)))
    {
      {
        p = 0;
        c = r;
        while ((c < (r + n)))
        {
          if ((p == r))
          {
            write("0 ");
            if ((c < n))
            {
              remember = c;
            } else
            {
              remember = ((c - n) + 1);
            }
          } else if (((p == (n - 1)) && (r > 0)))
          {
            write(remember);
            lastrow[r] = remember;
          } else
          {
            if ((c < n))
            {
              write(c, cpp_char(" "));
            } else
            {
              write(((c - n) + 1), cpp_char(" "));
            }
          }
          c += 1;
          p += 1;
        }
      }
      write(cpp_char("\n"));
      r += 1;
    }
  }
  {
    c = 0;
    while ((c < n))
    {
      write(lastrow[c], cpp_char(" "));
      c += 1;
    }
  }
  write(cpp_char("\n"));
  return 0;
}
