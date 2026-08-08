// Translated from solution.cpp.

func set_if_renew(current_diff: dynamic, current_t: dynamic, current_cards: dynamic, t: dynamic, cards: dynamic)
{
  if ((current_diff < abs((t - cards))))
  {
    current_diff = abs((t - cards));
    current_t = t;
    current_cards = cards;
  } else if (((current_diff == abs((t - cards))) && (current_t > t)))
  {
    current_t = t;
    current_cards = cards;
  }
}

class Card
{
  var value: dynamic;
  var index: dynamic;
}

func main()
{
  var n: dynamic;
  var k: dynamic;
  var a: dynamic;
  var b: dynamic;
  read(n, k, a, b);
  {
    var i = 0;
    while ((i < n))
    {
      read(cards[i].value);
      cards[i].index = i;
      i += 1;
    }
  }
  var sum = accumulate(cards.begin(), cards.end(), 0, __cpp_lambda_1);
  var max = max_element(cards.begin(), cards.end(), __cpp_lambda_2)->value;
  sort(cards.begin(), cards.end(), __cpp_lambda_3);
  var last_index: dynamic;
  {
    var i = 0;
    while ((i <= k))
    {
      last_index.emplace_back((max(b, (max * i)) + 1), INT_MAX);
      i += 1;
    }
  }
  var queue: dynamic;
  queue.push(0);
  last_index[0][0] = 0;
  {
    var count = 1;
    while ((count <= k))
    {
      {
        var c = queue.size();
        while ((c > 0))
        {
          var top = queue.front();
          queue.pop();
          {
            var i = last_index[(count - 1)][top];
            while (((i < n) && ((cards[i].value + top) < last_index[count].size())))
            {
              if ((last_index[count][(cards[i].value + top)] == INT_MAX))
              {
                queue.push((cards[i].value + top));
              }
              if ((last_index[count][(cards[i].value + top)] > i))
              {
                last_index[count][(cards[i].value + top)] = (i + 1);
              }
              i += 1;
            }
          }
          c -= 1;
        }
      }
      count += 1;
    }
  }
  var suspects: dynamic;
  while ((!queue.empty()))
  {
    suspects.push_back(queue.front());
    queue.pop();
  }
  sort(suspects.begin(), suspects.end());
  var max_diff = -1;
  var min_t = INT_MAX;
  var min_cards = 0;
  {
    var i = (lower_bound(suspects.begin(), suspects.end(), a) - suspects.begin());
    while ((i < suspects.size()))
    {
      if ((i == 0))
      {
        max_diff = (suspects[0] - a);
        min_t = a;
        min_cards = suspects[0];
      } else
      {
        if ((suspects[(i - 1)] >= b))
        {
          break;
        }
        var upper = suspects[i];
        var lower = suspects[(i - 1)];
        var diff = (upper - lower);
        if ((max_diff < (diff / 2)))
        {
          if (((lower <= a) && (a <= upper)))
          {
            if (((a - lower) < (upper - a)))
            {
              set_if_renew(max_diff, min_t, min_cards, a, lower);
            } else
            {
              set_if_renew(max_diff, min_t, min_cards, a, upper);
            }
          }
          if (((lower <= b) && (b <= upper)))
          {
            if (((b - lower) < (upper - b)))
            {
              set_if_renew(max_diff, min_t, min_cards, b, lower);
            } else
            {
              set_if_renew(max_diff, min_t, min_cards, b, upper);
            }
          }
          if (((a <= (lower + (diff / 2))) && ((lower + (diff / 2)) <= b)))
          {
            set_if_renew(max_diff, min_t, min_cards, (lower + (diff / 2)), lower);
          }
          if (((a <= (upper - (diff / 2))) && ((upper - (diff / 2)) <= b)))
          {
            set_if_renew(max_diff, min_t, min_cards, (upper - (diff / 2)), upper);
          }
        }
      }
      i += 1;
    }
  }
  if ((suspects.back() <= b))
  {
    set_if_renew(max_diff, min_t, min_cards, b, suspects.back());
  }
  write(min_t, "\n");
  var result: dynamic;
  {
    var i = k;
    while ((i > 0))
    {
      result.push_back((cards[(last_index[i][min_cards] - 1)].index + 1));
      min_cards = (min_cards - cards[(last_index[i][min_cards] - 1)].value);
      i -= 1;
    }
  }
  {
    var i = 0;
    while ((i < result.size()))
    {
      if ((i != 0))
      {
        write(cpp_char(" "));
      }
      write(result[i]);
      i += 1;
    }
  }
  write(cpp_char("\n"));
}

func __cpp_lambda_1(acc: dynamic, c: dynamic)
{
  return (acc + c.value);
}

func __cpp_lambda_2(a: dynamic, b: dynamic)
{
  return (a.value < b.value);
}

func __cpp_lambda_3(a: dynamic, b: dynamic)
{
  return (a.value < b.value);
}
