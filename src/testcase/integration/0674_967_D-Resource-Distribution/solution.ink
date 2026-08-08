// Translated from solution.cpp.

class node
{
  var index: dynamic;
  var num: dynamic;
  func node()
  {
    }
  func node(index: dynamic, num: dynamic)
  {
      index = index;
      num = num;
    }
}

func cmp(a: dynamic, b: dynamic)
{
  return (a.num < b.num);
}

func mylowerbound(vec: dynamic, n: dynamic)
{
  var start = 0;
  var end = (vec.size() - 1);
  if ((n < vec[0].num))
  {
    return 0;
  }
  if ((n > vec.back().num))
  {
    return vec.size();
  }
  while ((start <= end))
  {
    if ((start == end))
    {
      return start;
    } else if ((start == (end - 1)))
    {
      if ((vec[start].num >= n))
      {
        return start;
      }
      return end;
    } else
    {
      var mid = (start + (((end - start)) / 2));
      if ((vec[mid].num >= n))
      {
        end = mid;
      } else
      {
        start = (mid + 1);
      }
    }
  }
  return -1;
}

func main()
{
  var n = 0;
  var x1 = 0;
  var x2 = 0;
  read(n, x1, x2);
  var vec: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      var curnum = 0;
      scanf("%d", (&curnum));
      vec.push_back(node(i, curnum));
      i += 1;
    }
  }
  sort(vec.begin(), vec.end(), cmp);
  {
    var i = 1;
    while ((i <= n))
    {
      var curNum = ((((x1 + i) - 1)) / i);
      var index = mylowerbound(vec, curNum);
      var nextIndex = (i + index);
      if ((nextIndex < n))
      {
        curNum = vec[nextIndex].num;
        var len = ((((x2 + curNum) - 1)) / curNum);
        if ((len <= (n - nextIndex)))
        {
          write("Yes", "\n");
          write(i, " ", len, "\n");
          {
            var j = index;
            while ((j < nextIndex))
            {
              write((vec[j].index + 1), " ");
              j += 1;
            }
          }
          write("\n");
          {
            var j = 0;
            while ((j < len))
            {
              write((vec[(j + nextIndex)].index + 1), " ");
              j += 1;
            }
          }
          write("\n");
          return 0;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      var curNum = ((((x2 + i) - 1)) / i);
      var index = mylowerbound(vec, curNum);
      var nextIndex = (i + index);
      if ((nextIndex < n))
      {
        curNum = vec[nextIndex].num;
        var len = ((((x1 + curNum) - 1)) / curNum);
        if ((len <= (n - nextIndex)))
        {
          write("Yes", "\n");
          write(len, " ", i, "\n");
          {
            var j = 0;
            while ((j < len))
            {
              write((vec[(j + nextIndex)].index + 1), " ");
              j += 1;
            }
          }
          write("\n");
          {
            var j = index;
            while ((j < nextIndex))
            {
              write((vec[j].index + 1), " ");
              j += 1;
            }
          }
          write("\n");
          return 0;
        }
      }
      i += 1;
    }
  }
  write("No", "\n");
  return 0;
}
