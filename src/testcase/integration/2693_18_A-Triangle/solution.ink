// Translated from solution.cpp.

func distance(a: dynamic, b: dynamic, x: dynamic, y: dynamic)
{
  return (((((a - x)) * ((a - x))) + (((b - y)) * ((b - y)))));
}

func check(cod: dynamic)
{
  var d1: dynamic;
  var d2: dynamic;
  var d3: dynamic;
  var i = 0;
  {
    i = 0;
    while ((i < cod.size()))
    {
      cod[i] -= 1;
      d1 = distance(cod[0], cod[1], cod[2], cod[3]);
      d2 = distance(cod[0], cod[1], cod[4], cod[5]);
      d3 = distance(cod[2], cod[3], cod[4], cod[5]);
      cod[i] += 1;
      if (cpp_binary(cpp_binary((d1 == 0), "or", (d2 == 0)), "or", (d3 == 0)))
      {
        i += 1;
        continue;
      }
      if (cpp_binary(cpp_binary(((d1 == (d2 + d3))), "or", ((d2 == (d1 + d3)))), "or", ((d3 == (d2 + d1)))))
      {
        return 1;
      }
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < cod.size()))
    {
      cod[i] += 1;
      d1 = distance(cod[0], cod[1], cod[2], cod[3]);
      d2 = distance(cod[0], cod[1], cod[4], cod[5]);
      d3 = distance(cod[2], cod[3], cod[4], cod[5]);
      cod[i] -= 1;
      if (cpp_binary(cpp_binary((d1 == 0), "or", (d2 == 0)), "or", (d3 == 0)))
      {
        i += 1;
        continue;
      }
      if (cpp_binary(cpp_binary(((d1 == (d2 + d3))), "or", ((d2 == (d1 + d3)))), "or", ((d3 == (d2 + d1)))))
      {
        return 1;
      }
      i += 1;
    }
  }
  return 0;
}

func main()
{
  var cod: dynamic;
  var i: dynamic;
  {
    i = 0;
    while ((i < 6))
    {
      var temp: dynamic;
      read(temp);
      cod.push_back(temp);
      i += 1;
    }
  }
  var d1: dynamic;
  var d2: dynamic;
  var d3: dynamic;
  d1 = distance(cod[0], cod[1], cod[2], cod[3]);
  d2 = distance(cod[0], cod[1], cod[4], cod[5]);
  d3 = distance(cod[2], cod[3], cod[4], cod[5]);
  if (cpp_binary(cpp_binary(((d1 == (d2 + d3))), "or", ((d2 == (d1 + d3)))), "or", ((d3 == (d2 + d1)))))
  {
    write("RIGHT", "\n");
    return 0;
  } else if (check(cod))
  {
    write("ALMOST", "\n");
  } else
  {
    write("NEITHER", "\n");
  }
  return 0;
}
