// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  var A: dynamic;
  var B: dynamic;
  read(a, A);
  read(b, B);
  var chess = [0];
  {
    var i = 1;
    while ((i < 9))
    {
      chess[i][A] = 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < 9))
    {
      chess[(a - 96)][i] = 1;
      i += 1;
    }
  }
  if (((b - 98) > 0))
  {
    if (((B - 1) > 0))
    {
      chess[(b - 98)][(B - 1)] = 1;
    }
    if (((B + 1) < 9))
    {
      chess[(b - 98)][(B + 1)] = 1;
    }
  }
  if (((b - 94) < 9))
  {
    if (((B - 1) > 0))
    {
      chess[(b - 94)][(B - 1)] = 1;
    }
    if (((B + 1) < 9))
    {
      chess[(b - 94)][(B + 1)] = 1;
    }
  }
  if (((B - 2) > 0))
  {
    if (((b - 97) > 0))
    {
      chess[(b - 97)][(B - 2)] = 1;
    }
    if (((b - 95) < 9))
    {
      chess[(b - 95)][(B - 2)] = 1;
    }
  }
  if (((B + 2) < 9))
  {
    if (((b - 97) > 0))
    {
      chess[(b - 97)][(B + 2)] = 1;
    }
    if (((b - 95) < 9))
    {
      chess[(b - 95)][(B + 2)] = 1;
    }
  }
  if (((a - 97) > 0))
  {
    if (((A + 2) < 9))
    {
      chess[(a - 97)][(A + 2)] = 1;
    }
    if (((A - 2) > 0))
    {
      chess[(a - 97)][(A - 2)] = 1;
    }
  }
  if (((a - 95) < 9))
  {
    if (((A + 2) < 9))
    {
      chess[(a - 95)][(A + 2)] = 1;
    }
    if (((A - 2) > 0))
    {
      chess[(a - 95)][(A - 2)] = 1;
    }
  }
  if (((A - 1) > 0))
  {
    if (((a - 94) < 9))
    {
      chess[(a - 94)][(A - 1)] = 1;
    }
    if (((a - 98) > 0))
    {
      chess[(a - 98)][(A - 1)] = 1;
    }
  }
  if (((A + 1) < 9))
  {
    if (((a - 94) < 9))
    {
      chess[(a - 94)][(A + 1)] = 1;
    }
    if (((a - 98) > 0))
    {
      chess[(a - 98)][(A + 1)] = 1;
    }
  }
  chess[(b - 96)][B] = 1;
  var w = 0;
  {
    var i = 1;
    while ((i < 9))
    {
      {
        var j = 1;
        while ((j < 9))
        {
          if ((chess[i][j] == 0))
          {
            w += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(w);
  return 0;
}
