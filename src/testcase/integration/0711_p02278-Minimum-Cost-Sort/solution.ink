// Translated from solution.cpp.

func main(argument_0: dynamic) -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var W: dynamic = cpp_uninitialized();
  var MIN: dynamic = 100001;
  var sum: dynamic = 0;
  var p: dynamic = 0;
  var pp: dynamic = cpp_uninitialized();
  var cost: dynamic = 0;
  var k: dynamic = 0;
  var min: dynamic = 100001;
  read(n);
  var box: dynamic = cpp_uninitialized();
  var list: dynamic = cpp_uninitialized();
  var mark: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(W);
      box.push_back(W);
      list.push_back(W);
      mark.push_back(false);
      if ((W < MIN))
      {
        MIN = W;
      }
      i += 1;
    }
  }
  sort(list.begin(), list.end());
  while ((p < n))
  {
    if ((mark[p] == false))
    {
      sum = 0;
      k = 0;
      min = 100001;
      pp = p;
      while ((mark[pp] == false))
      {
        mark[pp] = true;
        sum += box[pp];
        k += 1;
        if ((box[pp] < min))
        {
          min = box[pp];
        }
        pp = (find(list.begin(), list.end(), box[pp]) - list.begin());
      }
      if (((sum + (((k - 2)) * min)) > ((sum + min) + (((k + 1)) * MIN))))
      {
        cost += ((sum + min) + (((k + 1)) * MIN));
      } else
      {
        cost += (sum + (((k - 2)) * min));
      }
      p += 1;
    } else
    {
      p += 1;
    }
  }
  write(cost, "\n");
  return 0;
}
