// Translated from solution.cpp.

var con: dynamic;

var a: dynamic;

var b: dynamic;

var m: dynamic;

func main()
{
  con = 0;
  read(a, b, m);
  if ((m == 0))
  {
    if (((a * b) > 0))
    {
      write(-1, "\n");
      return 0;
    } else
    {
      write(0, "\n");
      return 0;
    }
  } else if ((m > 0))
  {
    if ((a > b))
    {
      swap(a, b);
    }
    if (((a <= 0) && (b <= 0)))
    {
      write(-1, "\n");
      return 0;
    }
    if ((b >= m))
    {
      write(0, "\n");
      return 0;
    }
    if ((a < 0))
    {
      con = ceil(((a * -1.0) / b));
      a += (b * con);
    }
    if ((a > b))
    {
      swap(a, b);
    }
    while ((b < m))
    {
      var tmp = (a + b);
      a = b;
      b = tmp;
      con += 1;
    }
    write(con, "\n");
  } else if ((m < 0))
  {
    if (((a < m) && (b < m)))
    {
      write(-1, "\n");
      return 0;
    } else
    {
      write(0, "\n");
      return 0;
    }
  }
}
