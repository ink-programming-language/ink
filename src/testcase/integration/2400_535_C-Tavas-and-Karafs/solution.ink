// Translated from solution.cpp.

func imprimirVector(v: dynamic)
{
  if ((!v.empty()))
  {
    var p = v.size();
    write("[");
    {
      var i = 0;
      while ((i < cpp_cast(((p - 1)))))
      {
        write(v[i], ",");
        i += 1;
      }
    }
    write(v[(p - 1)], "]", "\n");
  } else
  {
    write("[]", "\n");
  }
}

func cuadratica(a: dynamic, b: dynamic, c: dynamic)
{
  var res = floor(((((-b) + sqrt(((b * b) - ((4 * a) * c))))) / ((2 * a))));
  return res;
}

func query(A: dynamic, B: dynamic, l: dynamic, t: dynamic, m: dynamic)
{
  if ((t < ((A + (((l - 1)) * B)))))
  {
    return -1;
  } else
  {
    return min(cuadratica(B, (((2 * A) - B)), ((((((2 * ((1 - l))) * ((A + (B * l)))) - (2 * B)) + ((B * l) * l)) + (B * l)) - ((2 * m) * t))), ((((t - A) + B)) / B));
  }
}

func main()
{
  var n: dynamic;
  var A: dynamic;
  var B: dynamic;
  read(A, B, n);
  {
    var i = 0;
    while ((i < cpp_cast((n))))
    {
      var l: dynamic;
      var t: dynamic;
      var m: dynamic;
      read(l, t, m);
      write(query(A, B, l, t, m), "\n");
      i += 1;
    }
  }
  return 0;
}
