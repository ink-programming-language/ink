// Translated from solution.cpp.

class Vector
{
  var x: dynamic;
  var y: dynamic;
  func Vector(x: dynamic = 0, y: dynamic = 0)
  {
      x = x;
      y = y;
    }
}

func operator_add(a: dynamic, b: dynamic)
{
  return Vector((a.x + b.x), (a.y + b.y));
}

func operator_subtract(a: dynamic, b: dynamic)
{
  return Vector((a.x - b.x), (a.y - b.y));
}

func operator_multiply(a: dynamic, b: dynamic)
{
  return ((a.x * b.x) + (a.y * b.y));
}

func cross(a: dynamic, b: dynamic)
{
  return ((a.x * b.y) - (a.y * b.x));
}

var a = cpp_array(4005);

var str = cpp_array(4005);

var mark = cpp_array(4005);

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i].x, a[i].y);
      i += 1;
    }
  }
  read((str + 1));
  str[0] = cpp_char("L");
  var i = 1;
  {
    var k = 2;
    while ((k <= n))
    {
      if ((a[k].x > a[i].x))
      {
        i = k;
      }
      k += 1;
    }
  }
  write(i, cpp_char(" "));
  mark[i] = 1;
  {
    var t = 1;
    while ((t < (n - 1)))
    {
      var j = 1;
      while (mark[j])
      {
        j += 1;
      }
      if ((str[t] == cpp_char("L")))
      {
        {
          var k = 1;
          while ((k <= n))
          {
            if (((!mark[k]) && (cross((a[j] - a[i]), (a[k] - a[i])) < 0)))
            {
              j = k;
            }
            k += 1;
          }
        }
      } else
      {
        {
          var k = 1;
          while ((k <= n))
          {
            if (((!mark[k]) && (cross((a[j] - a[i]), (a[k] - a[i])) > 0)))
            {
              j = k;
            }
            k += 1;
          }
        }
      }
      mark[cpp_assign(i, "=", j)] = 1;
      write(i, cpp_char(" "));
      t += 1;
    }
  }
  i = 1;
  while (mark[i])
  {
    i += 1;
  }
  write(i, cpp_char(" "));
  return 0;
}
